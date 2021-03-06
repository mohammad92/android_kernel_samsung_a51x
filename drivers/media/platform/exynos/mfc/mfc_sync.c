/*
 * drivers/media/platform/exynos/mfc/mfc_intr.c
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "mfc_sync.h"

#include "mfc_hw_reg_api.h"
#include "mfc_perf_measure.h"

#include "mfc_queue.h"

#define R2H_BIT(x)	(((x) > 0) ? (1 << ((x) - 1)) : 0)

static inline unsigned int __mfc_r2h_bit_mask(int cmd)
{
	unsigned int mask = R2H_BIT(cmd);

	if (cmd == MFC_REG_R2H_CMD_FRAME_DONE_RET)
		mask |= (R2H_BIT(MFC_REG_R2H_CMD_FIELD_DONE_RET) |
			 R2H_BIT(MFC_REG_R2H_CMD_COMPLETE_SEQ_RET) |
			 R2H_BIT(MFC_REG_R2H_CMD_SLICE_DONE_RET) |
			 R2H_BIT(MFC_REG_R2H_CMD_INIT_BUFFERS_RET) |
			 R2H_BIT(MFC_REG_R2H_CMD_ENC_BUFFER_FULL_RET));
	/* FIXME: Temporal mask for S3D SEI processing */
	else if (cmd == MFC_REG_R2H_CMD_INIT_BUFFERS_RET)
		mask |= (R2H_BIT(MFC_REG_R2H_CMD_FIELD_DONE_RET) |
			 R2H_BIT(MFC_REG_R2H_CMD_SLICE_DONE_RET) |
			 R2H_BIT(MFC_REG_R2H_CMD_FRAME_DONE_RET));

	return (mask |= R2H_BIT(MFC_REG_R2H_CMD_ERR_RET));
}

#define wait_condition(x, c) (x->int_condition &&		\
		(R2H_BIT(x->int_reason) & __mfc_r2h_bit_mask(c)))
#define is_err_cond(x)	((x->int_condition) && (x->int_reason == MFC_REG_R2H_CMD_ERR_RET))

/*
 * Return value description
 * 0: waked up before timeout
 * 1: failed to get the response for the command before timeout
*/
int mfc_wait_for_done_dev(struct mfc_dev *dev, int command)
{
	int ret, cmd;

	ret = wait_event_timeout(dev->cmd_wq,
			wait_condition(dev, command),
			msecs_to_jiffies(MFC_INT_TIMEOUT));
	if (ret == 0) {
		mfc_err_dev("Interrupt (dev->int_reason:%d, command:%d) timed out\n",
							dev->int_reason, command);
		cmd = mfc_check_risc2host(dev);
		if (cmd) {
			if (cmd == -1) {
				mfc_info_dev("interrupt is being processed\n");
				goto wait_done;
			}
			ret = wait_event_timeout(dev->cmd_wq,
					wait_condition(dev, command),
					msecs_to_jiffies(MFC_INT_TIMEOUT * MFC_INT_TIMEOUT_CNT));
			if (ret == 0) {
				mfc_err_dev("Timeout: MFC driver waited for upward of %dmsec\n",
						3 * MFC_INT_TIMEOUT);
			} else {
				goto wait_done;
			}
		}
		call_dop(dev, dump_and_stop_debug_mode, dev);
		return 1;
	}

wait_done:
	if (command != MFC_REG_R2H_CMD_COMPLETE_QUEUE_RET)
		mfc_clear_cmd_only();
	mfc_debug_dev(2, "Finished waiting (dev->int_reason:%d, command: %d)\n",
							dev->int_reason, command);
	return 0;
}

/*
 * Return value description
 *  0: waked up before timeout
 *  1: failed to get the response for the command before timeout
 * -1: got the error response for the command before timeout
*/
int mfc_wait_for_done_ctx(struct mfc_ctx *ctx, int command)
{
	struct mfc_dev *dev = ctx->dev;
	int ret, cmd;
	unsigned int timeout = MFC_INT_TIMEOUT;

	if (command == MFC_REG_R2H_CMD_CLOSE_INSTANCE_RET)
		timeout = MFC_INT_SHORT_TIMEOUT;

	ret = wait_event_timeout(ctx->cmd_wq,
			wait_condition(ctx, command),
			msecs_to_jiffies(timeout));
	if (ret == 0) {
		mfc_err_ctx("Interrupt (ctx->int_reason:%d, command:%d) timed out\n",
							ctx->int_reason, command);
		cmd = mfc_check_risc2host(dev);
		if (cmd) {
			if (cmd == -1) {
				mfc_info_ctx("interrupt is being processed\n");
				goto wait_done;
			}
			ret = wait_event_timeout(ctx->cmd_wq,
					wait_condition(ctx, command),
					msecs_to_jiffies(MFC_INT_TIMEOUT * MFC_INT_TIMEOUT_CNT));
			if (ret == 0) {
				mfc_err_ctx("Timeout: MFC driver waited for upward of %dmsec\n",
						3 * MFC_INT_TIMEOUT);
			} else {
				goto wait_done;
			}
		}
		call_dop(dev, dump_and_stop_debug_mode, dev);
		return 1;
	}

wait_done:
	if (is_err_cond(ctx)) {
		mfc_err_ctx("Finished (ctx->int_reason:%d, command: %d)\n",
				ctx->int_reason, command);
		mfc_err_ctx("But error (ctx->int_err:%d)\n", ctx->int_err);
		return -1;
	}

	mfc_debug(2, "Finished waiting (ctx->int_reason:%d, command: %d)\n",
							ctx->int_reason, command);
	return 0;
}

/* Wake up device wait_queue */
void mfc_wake_up_dev(struct mfc_dev *dev, unsigned int reason,
		unsigned int err)
{
	dev->int_condition = 1;
	dev->int_reason = reason;
	dev->int_err = err;
	wake_up(&dev->cmd_wq);
}

/* Wake up context wait_queue */
void mfc_wake_up_ctx(struct mfc_ctx *ctx, unsigned int reason,
		unsigned int err)
{
	ctx->int_condition = 1;
	ctx->int_reason = reason;
	ctx->int_err = err;
	wake_up(&ctx->cmd_wq);
}

int mfc_get_new_ctx(struct mfc_dev *dev)
{
	unsigned long wflags;
	int new_ctx_index = 0;
	int cnt = 0;
	int i;

	spin_lock_irqsave(&dev->work_bits.lock, wflags);

	mfc_debug_dev(2, "Previous context: %d (bits %08lx)\n", dev->curr_ctx,
							dev->work_bits.bits);

	if (dev->preempt_ctx > MFC_NO_INSTANCE_SET) {
		new_ctx_index = dev->preempt_ctx;
		mfc_debug_dev(2, "preempt_ctx is : %d\n", new_ctx_index);
	} else {
		for (i = 0; i < MFC_NUM_CONTEXTS; i++) {
			if (test_bit(i, &dev->otf_inst_bits)) {
				if (test_bit(i, &dev->work_bits.bits)) {
					spin_unlock_irqrestore(&dev->work_bits.lock, wflags);
					return i;
				}
				break;
			}
		}

		new_ctx_index = (dev->curr_ctx + 1) % MFC_NUM_CONTEXTS;
		while (!test_bit(new_ctx_index, &dev->work_bits.bits)) {
			new_ctx_index = (new_ctx_index + 1) % MFC_NUM_CONTEXTS;
			cnt++;
			if (cnt > MFC_NUM_CONTEXTS) {
				/* No contexts to run */
				spin_unlock_irqrestore(&dev->work_bits.lock, wflags);
				return -EAGAIN;
			}
		}
	}

	spin_unlock_irqrestore(&dev->work_bits.lock, wflags);
	return new_ctx_index;
}

int mfc_get_next_ctx(struct mfc_dev *dev)
{
	unsigned long wflags;
	int curr_ctx_index = dev->curr_ctx;
	int next_ctx_index = (curr_ctx_index + 1) % MFC_NUM_CONTEXTS;

	spin_lock_irqsave(&dev->work_bits.lock, wflags);

	mfc_debug_dev(2, "Current context: %d (bits %08lx)\n",
			dev->curr_ctx, dev->work_bits.bits);

	while (!test_bit(next_ctx_index, &dev->work_bits.bits)) {
		next_ctx_index = (next_ctx_index + 1) % MFC_NUM_CONTEXTS;
		if (next_ctx_index == curr_ctx_index) {
			/* No other context to run */
			spin_unlock_irqrestore(&dev->work_bits.lock, wflags);
			return -EAGAIN;
		}
	}

	spin_unlock_irqrestore(&dev->work_bits.lock, wflags);
	return next_ctx_index;
}

int __mfc_dec_ctx_ready_set_bit(struct mfc_ctx *ctx, struct mfc_bits *data, bool set)
{
	struct mfc_dev *dev = ctx->dev;
	int src_buf_queue_greater_than_0 = 0;
	int dst_buf_queue_greater_than_0 = 0;
	unsigned long flags;
	int is_ready = 0;

	mfc_debug(1, "[c:%d] src = %d, dst = %d, src_nal = %d, dst_nal = %d, state = %d, capstat = %d, waitstat = %d\n",
		  ctx->num, mfc_get_queue_count(&ctx->buf_queue_lock, &ctx->src_buf_queue),
		  mfc_get_queue_count(&ctx->buf_queue_lock, &ctx->dst_buf_queue),
		  mfc_get_queue_count(&ctx->buf_queue_lock, &ctx->src_buf_nal_queue),
		  mfc_get_queue_count(&ctx->buf_queue_lock, &ctx->dst_buf_nal_queue),
		  ctx->state, ctx->capture_state, ctx->wait_state);

	src_buf_queue_greater_than_0
		= mfc_is_queue_count_greater(&ctx->buf_queue_lock, &ctx->src_buf_queue, 0);
	dst_buf_queue_greater_than_0
		= mfc_is_queue_count_greater(&ctx->buf_queue_lock, &ctx->dst_buf_queue, 0);

	/* If shutdown is called, do not try any cmd */
	if (dev->shutdown)
		return 0;

	/* The ready condition check and set work_bit should be synchronized */
	spin_lock_irqsave(&data->lock, flags);

	/* Context is to parse header */
	if (ctx->state == MFCINST_GOT_INST &&
		src_buf_queue_greater_than_0)
		is_ready = 1;

	/* Context is to decode a frame */
	else if (ctx->state == MFCINST_RUNNING &&
		ctx->wait_state == WAIT_NONE && src_buf_queue_greater_than_0 &&
		dst_buf_queue_greater_than_0)
		is_ready = 1;

	/* Context is to return last frame */
	else if (ctx->state == MFCINST_FINISHING && dst_buf_queue_greater_than_0)
		is_ready = 1;

	/* Context is to set buffers */
	else if (ctx->state == MFCINST_HEAD_PARSED &&
		(dst_buf_queue_greater_than_0 && ctx->wait_state == WAIT_NONE))
		is_ready = 1;

	/* Resolution change */
	else if ((ctx->state == MFCINST_RES_CHANGE_INIT || ctx->state == MFCINST_RES_CHANGE_FLUSH) &&
		dst_buf_queue_greater_than_0)
		is_ready = 1;

	else if (ctx->state == MFCINST_RES_CHANGE_END &&
		src_buf_queue_greater_than_0)
		is_ready = 1;

	if ((is_ready == 1) && (set == true)) {
		/* if the ctx is ready and request set_bit, set the work_bit */
		__set_bit(ctx->num, &data->bits);
	} else if ((is_ready == 0) && (set == false)) {
		/* if the ctx is not ready and request clear_bit, clear the work_bit */
		__clear_bit(ctx->num, &data->bits);
	} else {
		if (set == true) {
			/* If the ctx is not ready, this is not included to S/W driver margin */
			mfc_perf_cancel_drv_margin(dev);
			mfc_debug(2, "ctx is not ready\n");
		}
	}

	spin_unlock_irqrestore(&data->lock, flags);

	return is_ready;
}

static int __mfc_enc_ctx_ready_set_bit(struct mfc_ctx *ctx, struct mfc_bits *data, bool set)
{
	struct mfc_enc *enc = ctx->enc_priv;
	struct mfc_dev *dev = ctx->dev;
	struct mfc_enc_params *p = &enc->params;
	int src_buf_queue_greater_than_0 = 0;
	int dst_buf_queue_greater_than_0 = 0;
	unsigned long flags;
	int is_ready = 0;

	mfc_debug(1, "[c:%d] src = %d, dst = %d, src_nal = %d, dst_nal = %d, ref = %d, state = %d\n",
		  ctx->num, mfc_get_queue_count(&ctx->buf_queue_lock, &ctx->src_buf_queue),
		  mfc_get_queue_count(&ctx->buf_queue_lock, &ctx->dst_buf_queue),
		  mfc_get_queue_count(&ctx->buf_queue_lock, &ctx->src_buf_nal_queue),
		  mfc_get_queue_count(&ctx->buf_queue_lock, &ctx->dst_buf_nal_queue),
		  mfc_get_queue_count(&ctx->buf_queue_lock, &ctx->ref_buf_queue),
		  ctx->state);

	src_buf_queue_greater_than_0
		= mfc_is_queue_count_greater(&ctx->buf_queue_lock, &ctx->src_buf_queue, 0);
	dst_buf_queue_greater_than_0
		= mfc_is_queue_count_greater(&ctx->buf_queue_lock, &ctx->dst_buf_queue, 0);

	/* If shutdown is called, do not try any cmd */
	if (dev->shutdown)
		return 0;

	/* The ready condition check and set work_bit should be synchronized */
	spin_lock_irqsave(&data->lock, flags);

	/* context is ready to make header */
	if (ctx->state == MFCINST_GOT_INST &&
		dst_buf_queue_greater_than_0) {
		if (p->seq_hdr_mode == V4L2_MPEG_VIDEO_HEADER_MODE_AT_THE_READY) {
			if (src_buf_queue_greater_than_0)
				is_ready = 1;
		} else {
			is_ready = 1;
		}
	}

	/* context is ready to allocate DPB */
	else if (ctx->state == MFCINST_HEAD_PARSED &&
		dst_buf_queue_greater_than_0)
		is_ready = 1;

	/* context is ready to encode a frame */
	else if (ctx->state == MFCINST_RUNNING &&
		src_buf_queue_greater_than_0 && dst_buf_queue_greater_than_0)
		is_ready = 1;

	/* context is ready to encode a frame for NAL_ABORT command */
	else if (ctx->state == MFCINST_ABORT_INST &&
		src_buf_queue_greater_than_0 && dst_buf_queue_greater_than_0)
		is_ready = 1;

	/* context is ready to encode remain frames */
	else if (ctx->state == MFCINST_FINISHING &&
		src_buf_queue_greater_than_0 && dst_buf_queue_greater_than_0)
		is_ready = 1;

	if ((is_ready == 1) && (set == true)) {
		/* if the ctx is ready and request set_bit, set the work_bit */
		__set_bit(ctx->num, &data->bits);
	} else if ((is_ready == 0) && (set == false)) {
		/* if the ctx is not ready and request clear_bit, clear the work_bit */
		__clear_bit(ctx->num, &data->bits);
	} else {
		if (set == true) {
			mfc_perf_cancel_drv_margin(dev);
			mfc_debug(2, "ctx is not ready\n");
		}
	}

	spin_unlock_irqrestore(&data->lock, flags);

	return is_ready;
}


int mfc_ctx_ready_set_bit(struct mfc_ctx *ctx, struct mfc_bits *data)
{
	if (ctx->type == MFCINST_DECODER)
		return __mfc_dec_ctx_ready_set_bit(ctx, data, true);
	else if (ctx->type == MFCINST_ENCODER)
		return __mfc_enc_ctx_ready_set_bit(ctx, data, true);

	return 0;
}

int mfc_ctx_ready_clear_bit(struct mfc_ctx *ctx, struct mfc_bits *data)
{
	if (ctx->type == MFCINST_DECODER)
		return __mfc_dec_ctx_ready_set_bit(ctx, data, false);
	else if (ctx->type == MFCINST_ENCODER)
		return __mfc_enc_ctx_ready_set_bit(ctx, data, false);

	return 0;
}
