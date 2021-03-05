ARCH=arm64
CROSS_COMPILE=aarch64-linux-android-
CLANG_TRIPLE=aarch64-linux-gnu-
DEFCONFIG=m2_defconfig
CLANG_VERSION=clang-r353983c
TARGET_SOC=exynos9630
TARGET_DTBO_CFG=exynos9630_m2_dtboimg.cfg
SEPERATE_KERNEL_OBJ=true
FILES="
arch/arm64/boot/Image:kernel
arch/arm64/boot/dts/exynos/exynos9630_m2.dtb:dtb.img
dtbo.img:dtbo.img
vmlinux:vmlinux
"
