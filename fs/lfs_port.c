#include "lfs.h"
#include "sfud.h"

struct lfs_config cfg; // lfs 文件系统配置结构体


int lfs_spi_flash_init(struct lfs_config *cfg);
int lfs_spi_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_spi_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_spi_flash_erase(const struct lfs_config *cfg, lfs_block_t block);
int lfs_spi_flash_sync(const struct lfs_config *cfg);

int lfs_spi_flash_init(struct lfs_config *cfg) {

        cfg->read = lfs_spi_flash_read;
        cfg->prog = lfs_spi_flash_prog;
        cfg->erase = lfs_spi_flash_erase;
        cfg->sync = lfs_spi_flash_sync;
 
        // 最小读取字节数，所有的读取操作字节数必须是它的整数倍
        cfg->read_size = 16;
        // 最小写入字节数，所有的写入操作字节数必须是它的整数倍
        cfg->prog_size = 16;
        // 擦除块操作的字节数，该选项不影响 RAM 消耗，可以比物理擦除尺寸大
        // 但是每个文件至少占用一个块，必须是读取和写入操作字节数的整数倍
        cfg->block_size = 4096;
        // 设备上可擦除块的数量，即容量
        cfg->block_count = 4096;
        // littlefs 系统删除元数据日志并将元数据移动到另一个块之前的擦除周期数。
        // 建议取值范围为 100 ~ 1000，较大数值有较好的性能但是会导致磨损分布不一致
        // 取值 -1 的话，即为禁用块级磨损均衡
        cfg->block_cycles = 500;
        // 块缓存大小，每个缓存都会在 RAM 中缓冲一部分块数据，
        // littlefs 系统需要一个读取缓存、一个写入缓存，每个文件还需要一个额外的缓存。
        // 更大的缓存可以通过存储更多的数据并降低磁盘访问数量等手段来提高性能
        cfg->cache_size = 16;
        // 先行缓冲大小，更大的先行缓冲可以提高分配操作中可被发现的块数量
        // 即分配块时每次步进多少个块，16就表示每次分配16个块
        // 先行缓冲以紧凑的bit位形式来存储，故 RAM 中的一个字节可以对应8个块
        // 该值必须是8的整数倍
        cfg->lookahead_size = 16;
        return LFS_ERR_OK;

}

int lfs_spi_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    // check if read is valid
		sfud_flash *flash_dev = sfud_get_device(0);
    sfud_read(flash_dev, block * cfg->block_size + off, size, (uint8_t *)buffer);
    return LFS_ERR_OK;
}
int lfs_spi_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    // check if write is valid
		sfud_flash *flash_dev = sfud_get_device(0);
    sfud_write(flash_dev, block * cfg->block_size + off, size, (uint8_t *)buffer);
    return LFS_ERR_OK;
}

/*
 * @brief 擦除指定块。块在写入之前必须先被擦除过，被擦除块的状态是未定义
 * @param [in] lfs_config格式参数
 * @param [in] block 要擦除的逻辑块索引号，从0开始
 * @retval 0 成功, < 0 错误码
 */
int lfs_spi_flash_erase(const struct lfs_config *cfg, lfs_block_t block) {
    // check if erase is valid
    LFS_ASSERT(block < cfg->block_count);
		sfud_flash *flash_dev = sfud_get_device(0);
		sfud_erase(flash_dev, block * cfg->block_size, 4096);
    return LFS_ERR_OK;
}

/*
 * @brief 对底层块设备做同步操作。若底层块设备不没有同步这项操作可以直接返回
 * @param [in] lfs_config格式参数;
 * @retval 0 成功, < 0 错误码
 */
int lfs_spi_flash_sync(const struct lfs_config *cfg) {
    return LFS_ERR_OK;
}
