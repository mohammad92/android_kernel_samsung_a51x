/*
 * linux/drivers/gpu/exynos/g2d/g2d_debug.c
 *
 * Copyright (C) 2017 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/sched/task.h>

#include <media/exynos_tsmux.h>

#include "g2d.h"
#include "g2d_task.h"
#include "g2d_uapi.h"
#include "g2d_debug.h"
#include "g2d_regs.h"

#include <soc/samsung/exynos-devfreq.h>

unsigned int g2d_debug;

#define G2D_MAX_STAMP_ID 1024
#define G2D_STAMP_CLAMP_ID(id) ((id) & (G2D_MAX_STAMP_ID - 1))

static struct g2d_stamp {
	ktime_t time;
	unsigned long state;
	unsigned short stamp;
	unsigned char job_id;
	unsigned char cpu;
	union {
		u32 val[3];
		struct {
			char name[8];
			unsigned int seqno;
		} fence;
	};
} g2d_stamp_list[G2D_MAX_STAMP_ID];

static atomic_t g2d_stamp_id = ATOMIC_INIT(-1);

enum {
	G2D_STAMPTYPE_NONE,
	G2D_STAMPTYPE_NUM,
	G2D_STAMPTYPE_HEX,
	G2D_STAMPTYPE_USEC,
	G2D_STAMPTYPE_PERF,
	G2D_STAMPTYPE_INOUT,
	G2D_STAMPTYPE_ALLOCFREE,
	G2D_STAMPTYPE_FENCE,
};
static struct g2d_stamp_type {
	const char *name;
	int type;
	bool task_specific;
} g2d_stamp_types[G2D_STAMP_STATE_NUM] = {
	{"runtime_pm",    G2D_STAMPTYPE_INOUT,     false},
	{"task_alloc",    G2D_STAMPTYPE_ALLOCFREE, true},
	{"task_begin",    G2D_STAMPTYPE_NUM,       true},
	{"task_push",     G2D_STAMPTYPE_PERF,      true},
	{"irq",           G2D_STAMPTYPE_HEX,       false},
	{"task_done",     G2D_STAMPTYPE_USEC,      true},
	{"fence_timeout", G2D_STAMPTYPE_HEX,       true},
	{"hw_timeout",    G2D_STAMPTYPE_HEX,       true},
	{"irq_error",     G2D_STAMPTYPE_HEX,       true},
	{"mmu_fault",     G2D_STAMPTYPE_NONE,      true},
	{"shutdown",      G2D_STAMPTYPE_INOUT,     false},
	{"suspend",       G2D_STAMPTYPE_INOUT,     false},
	{"resume",        G2D_STAMPTYPE_INOUT,     false},
	{"hwfc_job",      G2D_STAMPTYPE_NUM,       true},
	{"pending",       G2D_STAMPTYPE_NUM,       false},
	{"fence",         G2D_STAMPTYPE_FENCE,     true},
	{"reset_task",    G2D_STAMPTYPE_NONE,      true},
};

static bool g2d_stamp_show_single(struct seq_file *s, struct g2d_stamp *stamp)
{
	if (stamp->time == 0)
		return false;

	seq_printf(s, "[%u:%12lld] %13s: ", stamp->cpu,
		   ktime_to_us(stamp->time),
		   g2d_stamp_types[stamp->stamp].name);

	if (g2d_stamp_types[stamp->stamp].task_specific)
		seq_printf(s, "JOB ID %2u (STATE %#05lx) - ",
			   stamp->job_id, stamp->state);

	switch (g2d_stamp_types[stamp->stamp].type) {
	case G2D_STAMPTYPE_NUM:
		seq_printf(s, "%u", stamp->val[0]);
		break;
	case G2D_STAMPTYPE_HEX:
		seq_printf(s, "%#x", stamp->val[0]);
		break;
	case G2D_STAMPTYPE_USEC:
		seq_printf(s, "%u usec.", stamp->val[0]);
		break;
	case G2D_STAMPTYPE_PERF:
		seq_printf(s, "%u usec. (INT %u MIF %u)", stamp->val[0],
			   stamp->val[1], stamp->val[2]);
		break;
	case G2D_STAMPTYPE_INOUT:
		seq_printf(s, "%s", stamp->val[0] ? "out" : "in");
		break;
	case G2D_STAMPTYPE_ALLOCFREE:
		seq_printf(s, "%s", stamp->val[0] ? "free" : "alloc");
		break;
	case G2D_STAMPTYPE_FENCE:
		seq_printf(s, "%u@%s", stamp->fence.seqno, stamp->fence.name);
		break;
	}

	seq_puts(s, "\n");

	return true;
}

static int g2d_stamp_show(struct seq_file *s, void *unused)
{
	int idx = G2D_STAMP_CLAMP_ID(atomic_read(&g2d_stamp_id) + 1);
	int i;

	/* in chronological order */
	for (i = idx; i < G2D_MAX_STAMP_ID; i++)
		if (!g2d_stamp_show_single(s, &g2d_stamp_list[i]))
			break;

	for (i = 0; i < idx; i++)
		if (!g2d_stamp_show_single(s, &g2d_stamp_list[i]))
			break;

	return 0;
}

static int g2d_debug_logs_open(struct inode *inode, struct file *file)
{
	return single_open(file, g2d_stamp_show, inode->i_private);
}

static const struct file_operations g2d_debug_logs_fops = {
	.open = g2d_debug_logs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int g2d_debug_contexts_show(struct seq_file *s, void *unused)
{
	struct g2d_device *g2d_dev = s->private;
	struct g2d_context *ctx;

	seq_printf(s, "%16s %6s %4s %6s %10s %10s %10s\n",
		"task", "pid", "prio", "dev", "rbw", "wbw", "devfreq");
	seq_puts(s, "------------------------------------------------------\n");

	spin_lock(&g2d_dev->lock_ctx_list);

	list_for_each_entry(ctx, &g2d_dev->ctx_list, node) {
		task_lock(ctx->owner);
		seq_printf(s, "%16s %6u %4d %6s %10llu %10llu %10u\n",
			ctx->owner->comm, ctx->owner->pid, ctx->priority,
			g2d_dev->misc[(ctx->authority + 1) & 1].name,
			ctx->ctxqos.rbw, ctx->ctxqos.wbw,
			ctx->ctxqos.devfreq);

		task_unlock(ctx->owner);
	}

	spin_unlock(&g2d_dev->lock_ctx_list);

	seq_puts(s, "------------------------------------------------------\n");

	seq_puts(s, "priorities:\n");
	seq_printf(s, "\tlow(0)    : %d\n",
		   atomic_read(&g2d_dev->prior_stats[G2D_LOW_PRIORITY]));
	seq_printf(s, "\tmedium(1) : %d\n",
		   atomic_read(&g2d_dev->prior_stats[G2D_MEDIUM_PRIORITY]));
	seq_printf(s, "\thigh(2)   : %d\n",
		   atomic_read(&g2d_dev->prior_stats[G2D_HIGH_PRIORITY]));
	seq_printf(s, "\thighest(3): %d\n",
		   atomic_read(&g2d_dev->prior_stats[G2D_HIGHEST_PRIORITY]));

	return 0;
}

static int g2d_debug_contexts_open(struct inode *inode, struct file *file)
{
	return single_open(file, g2d_debug_contexts_show, inode->i_private);
}

static const struct file_operations g2d_debug_contexts_fops = {
	.open = g2d_debug_contexts_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int g2d_debug_tasks_show(struct seq_file *s, void *unused)
{
	struct g2d_device *g2d_dev = s->private;
	struct g2d_task *task;

	for (task = g2d_dev->tasks; task; task = task->next) {
		seq_printf(s, "TASK[%d]: state %#lx flags %#x ",
			   task->sec.job_id, task->state, task->flags);
		seq_printf(s, "prio %d begin@%llu end@%llu nr_src %d ",
			   task->sec.priority, ktime_to_us(task->ktime_begin),
			   ktime_to_us(task->ktime_end), task->num_source);
		seq_printf(s, "rbw %llu wbw %llu devfreq %u\n",
			   task->taskqos.rbw, task->taskqos.wbw,
			   task->taskqos.devfreq);
	}

	return 0;
}

static int g2d_debug_tasks_open(struct inode *inode, struct file *file)
{
	return single_open(file, g2d_debug_tasks_show, inode->i_private);
}

static const struct file_operations g2d_debug_tasks_fops = {
	.open = g2d_debug_tasks_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

void g2d_init_debug(struct g2d_device *g2d_dev)
{
	g2d_dev->debug_root = debugfs_create_dir("g2d", NULL);
	if (!g2d_dev->debug_root) {
		perrdev(g2d_dev, "debugfs: failed to create root directory");
		return;
	}

	g2d_dev->debug = debugfs_create_u32("debug",
					0644, g2d_dev->debug_root, &g2d_debug);
	if (!g2d_dev->debug) {
		perrdev(g2d_dev, "debugfs: failed to create debug file");
		return;
	}

	g2d_dev->debug_logs = debugfs_create_file("logs",
					0444, g2d_dev->debug_root, g2d_dev, &g2d_debug_logs_fops);
	if (!g2d_dev->debug_logs) {
		perrdev(g2d_dev, "debugfs: failed to create logs file");
		return;
	}

	g2d_dev->debug_contexts = debugfs_create_file("contexts",
					0400, g2d_dev->debug_root, g2d_dev,
					&g2d_debug_contexts_fops);
	if (!g2d_dev->debug_logs) {
		perrdev(g2d_dev, "debugfs: failed to create contexts file");
		return;
	}

	g2d_dev->debug_tasks= debugfs_create_file("tasks",
					0400, g2d_dev->debug_root, g2d_dev,
					&g2d_debug_tasks_fops);
	if (!g2d_dev->debug_logs) {
		perrdev(g2d_dev, "debugfs: failed to create tasks file");
		return;
	}
}

void g2d_destroy_debug(struct g2d_device *g2d_dev)
{
	debugfs_remove_recursive(g2d_dev->debug_root);
}

static struct regs_info g2d_layer_info[] = {
		/* Start, Size, Name */
		{ 0x0,		0x20,	"General" },
		{ 0x34,		0x10,	"Secure Layer" },
		{ 0xF0,		0x10,	"AFBC debugging" },
		{ 0x80,		0x70,	"Job manager" },
		{ 0x8000,	0x100,	"HW flow control" },
		{ 0x120,	0xE0,	"Destination" },
		{ 0x200,	0x100,	"Layer0" },
		{ 0x300,	0x100,	"Layer1" },
		{ 0x400,	0x100,	"Layer2" },
		{ 0x500,	0x100,	"Layer3" },
		{ 0x600,	0x100,	"Layer4" },
		{ 0x700,	0x100,	"Layer5" },
		{ 0x800,	0x100,	"Layer6" },
		{ 0x900,	0x100,	"Layer7" },
		{ 0xA00,	0x100,	"Layer8" },
		{ 0xB00,	0x100,	"Layer9" },
		{ 0xC00,	0x100,	"Layer10" },
		{ 0xD00,	0x100,	"Layer11" },
		{ 0xE00,	0x100,	"Layer12" },
		{ 0xF00,	0x100,	"Layer13" },
		{ 0x1000,	0x100,	"Layer14" },
		{ 0x1100,	0x100,	"Layer15" },
};

static struct regs_info g2d_reg_hdr_info[] = {
		/* Start, Size, Name */
		{ 0x2000,	0x120,	"Layer CSC Coefficient" },
		{ 0x3000,	0x110,	"HDR EOTF" },
		{ 0x3200,	0x110,	"DEGAMMA EOTF" },
		{ 0x3400,	0x30,	"HDR GM" },
		{ 0x3500,	0x30,	"DEGAMMA 2.2" },
		{ 0x3600,	0x90,	"HDR TM" },
		{ 0x3700,	0x90,	"DEGAMMA TM" },
};

static struct regs_info g2d_reg_hdr10p_info[] = {
		/* Start, Size, Name */
		{ 0x3000,	0x8,	"COM_CTRL" },
		{ 0x3008,	0x8c,	"L0 OETF" },
		{ 0x3094,	0x308,	"L0 EOTF" },
		{ 0x339c,	0x24,	"L0 GM" },
		{ 0x3430,	0xE4,	"L0 TM" },
		{ 0x3808,	0x8c,	"L1 OETF" },
		{ 0x3894,	0x308,	"L1 EOTF" },
		{ 0x3b9c,	0x24,	"L1 GM" },
		{ 0x3c30,	0xE4,	"L1 TM" },
		{ 0x4008,	0x8c,	"L2 OETF" },
		{ 0x4094,	0x308,	"L2 EOTF" },
		{ 0x439c,	0x24,	"L2 GM" },
		{ 0x4430,	0xE4,	"L2 TM" },
		{ 0x4808,	0x8c,	"L3 OETF" },
		{ 0x4894,	0x308,	"L3 EOTF" },
		{ 0x4b9c,	0x24,	"L3 GM" },
		{ 0x4c30,	0xE4,	"L3 TM" },
};

#define G2D_COMP_DEBUG_DATA_COUNT 16

void g2d_dump_afbcdata(struct g2d_device *g2d_dev)
{
	int i, cluster;
	u32 cfg;

	for (cluster = 0; cluster < 2; cluster++) {
		for (i = 0; i < G2D_COMP_DEBUG_DATA_COUNT; i++) {
			writel_relaxed(i | cluster << 5,
				g2d_dev->reg + G2D_COMP_DEBUG_ADDR_REG);
			cfg = readl_relaxed(
				g2d_dev->reg + G2D_COMP_DEBUG_DATA_REG);

			pr_err("AFBC_DEBUGGING_DATA cluster%d [%d] %#x",
				cluster, i, cfg);
		}
	}
}

void g2d_dump_layer_sfr(struct g2d_device *g2d_dev, struct g2d_task *task)
{
	unsigned int i, num_array;

	num_array = (unsigned int)ARRAY_SIZE(g2d_layer_info) - g2d_dev->max_layers
		    + ((task) ? task->num_source : g2d_dev->max_layers);

	for (i = 0; i < num_array; i++) {
		pr_info("[%s: %04X .. %04X]\n",
			g2d_layer_info[i].name, g2d_layer_info[i].start,
			g2d_layer_info[i].start + g2d_layer_info[i].size);
		print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS, 32, 4,
			g2d_dev->reg + g2d_layer_info[i].start,
			g2d_layer_info[i].size, false);
	}
}

void g2d_dump_coeff_sfr(struct g2d_device *g2d_dev, struct g2d_task *task)
{
	unsigned int i;
	struct regs_info *info = (g2d_dev->caps & G2D_DEVICE_CAPS_HDR10P) ?
		g2d_reg_hdr10p_info : g2d_reg_hdr_info;
	unsigned int num_array = (g2d_dev->caps & G2D_DEVICE_CAPS_HDR10P) ?
		ARRAY_SIZE(g2d_reg_hdr10p_info) : ARRAY_SIZE(g2d_reg_hdr_info);

	for (i = 0; i < num_array; i++) {
		pr_info("[%s: %04X .. %04X]\n",
			info[i].name, info[i].start,
			info[i].start + info[i].size);
		print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS, 32, 4,
			g2d_dev->reg + info[i].start, info[i].size, false);
	}
}


void g2d_dump_cmd(struct g2d_task *task)
{
	unsigned int i;
	struct g2d_reg *regs;

	if (!task)
		return;

	regs = page_address(task->cmd_page);

	for (i = 0; i < task->sec.cmd_count; i++)
		pr_info("G2D: CMD[%03d] %#06x, %#010x\n",
			i, regs[i].offset, regs[i].value);
}

/*
 * If it happens error interrupts, mmu errors, and H/W timeouts,
 * dump the SFR and job command list of task, AFBC debugging information
 */
void g2d_dump_info(struct g2d_device *g2d_dev, struct g2d_task *task)
{
	g2d_dump_cmd(task);
	g2d_dump_layer_sfr(g2d_dev, task);
	g2d_dump_coeff_sfr(g2d_dev, task);
	g2d_dump_afbcdata(g2d_dev);
}

#ifdef CONFIG_G2D_SYSTRACE

#define CREATE_TRACE_POINTS
#include <trace/events/systrace.h>

static bool g2d_systrace_on;
module_param(g2d_systrace_on, bool, 0644);

#define G2D_TRACE_BUFSIZE 32
static void g2d_tracing_mark_write(struct g2d_task *task, u32 stampid, s32 val)
{
	char buffer[G2D_TRACE_BUFSIZE];
	int pid = 0;

	if (!g2d_systrace_on)
		return;

	switch (stampid) {
	case G2D_STAMP_STATE_TASK_RESOURCE:

		if (val)
			snprintf(buffer, G2D_TRACE_BUFSIZE, "C|%d|%s#%d|0",
				 pid, "g2d_frame_run", task->sec.job_id);
		else
			snprintf(buffer, G2D_TRACE_BUFSIZE, "C|%d|%s#%d|1",
				 pid, "g2d_frame_wait", task->sec.job_id);

		trace_tracing_mark_write(buffer);
	break;
	case G2D_STAMP_STATE_PUSH:
		snprintf(buffer, G2D_TRACE_BUFSIZE, "C|%d|%s#%d|0",
			 pid, "g2d_frame_wait", task->sec.job_id);

		trace_tracing_mark_write(buffer);

		snprintf(buffer, G2D_TRACE_BUFSIZE, "C|%d|%s#%d|1",
				 pid, "g2d_frame_run", task->sec.job_id);

		trace_tracing_mark_write(buffer);
	break;
	}
}
#else
#define g2d_tracing_mark_write(task, stampid, val)	do { } while (0)
#endif
void g2d_stamp_task(struct g2d_task *task, u32 stampid, u64 val)
{
	int idx = G2D_STAMP_CLAMP_ID(atomic_inc_return(&g2d_stamp_id));
	struct g2d_stamp *stamp = &g2d_stamp_list[idx];

	g2d_tracing_mark_write(task, stampid, val);

	if (task) {
		stamp->state = task->state;
		stamp->job_id = task->sec.job_id;
	} else {
		stamp->job_id = 0;
		stamp->state = 0;
	}

	if (g2d_stamp_types[stampid].type == G2D_STAMPTYPE_FENCE) {
		struct dma_fence *fence = (struct dma_fence *)val;

		strlcpy(stamp->fence.name, fence->ops->get_driver_name(fence),
			sizeof(stamp->fence.name));
		stamp->fence.seqno = fence->seqno;
	} else {
		stamp->val[0] = (u32)val;

		if (g2d_stamp_types[stampid].type == G2D_STAMPTYPE_PERF) {
			struct g2d_device *g2d_dev = task->g2d_dev;

			stamp->val[1] =	exynos_devfreq_get_domain_freq(
					g2d_dev->dvfs_int);
			stamp->val[2] = exynos_devfreq_get_domain_freq(
					g2d_dev->dvfs_mif);
		}
	}

	if (stampid == G2D_STAMP_STATE_DONE) {
		struct g2d_device *g2d_dev = task->g2d_dev;

		if (g2d_debug & (1 << DBG_DEBUG))
			g2d_dump_info(g2d_dev, task);

		g2d_info("Task %2d consumed %10lu us (int %lu mif %lu)\n",
			 stamp->job_id, (unsigned long)val,
			 exynos_devfreq_get_domain_freq(g2d_dev->dvfs_int),
			 exynos_devfreq_get_domain_freq(g2d_dev->dvfs_mif));
	}

	stamp->time = ktime_get();
	stamp->stamp = stampid;
	stamp->cpu = raw_smp_processor_id();

	/* LLWFD latency measure */
	/* media/exynos_tsmux.h includes below functions */
	if (task != NULL && IS_HWFC(task->flags)) {
		if (stampid == G2D_STAMP_STATE_PUSH)
			g2d_blending_start(task->sec.job_id);
		if (stampid == G2D_STAMP_STATE_DONE)
			g2d_blending_end(task->sec.job_id);
	}
}
