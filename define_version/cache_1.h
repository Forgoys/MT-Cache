typedef struct _icache1
{
    addrl curEaddr;                                                 // 当前缓存块的地址
    char bbuff[((1 << CLINE) + 128)] __attribute__((aligned(128))); // 缓存数据缓冲区
    int datasize;                                                   // 数据大小（字节）
    unsigned long fore_datasize;                                    // 缓存前部数据大小
    int buffsize;                                                   // 缓存大小
    int position;                                                   // 缓存位置
    unsigned long getsize;                                          // 获取数据大小
    int tag;                                                        // 缓存标签
    unsigned long begin, end;                                       // 缓存起始和结束地址
    unsigned long accessed, st_accessed;                            // 访问计数器
    unsigned long miss, st_miss;                                    // 未命中计数器
    unsigned long replace, st_replace;                              // 替换计数器
    unsigned long write_back, flush;                                // 写回和刷新计数器
    int modified, writable;                                         // 修改和可写标志
} i_cache1;

/**
 * ds_point_p：指向数据的指针，用于数据访问。
 * ds_EaAlign：对齐后的地址，用于确保数据对齐。
 * _caches_miss：缓存未命中次数。
 * ds_statsf, ds_st_statsf：缓存命中率，ds_st_statsf用于统计。
 * ds_buffer_size：缓存大小。
 * ds_hit, ds_tot, ds_mis：命中、总访问次数和未命中次数。
 * ds_st_hit, ds_st_tot, ds_st_mis：命中、总访问次数和未命中次数，用于统计。
 * little：小偏移量，用于计算缓存块内的偏移。
*/

#define CACHEs_ENV()                     \
    char *ds_point_p;                    \
    char *ds_EaAlign;                    \
    int _caches_miss;                    \
    double ds_statsf, ds_st_statsf;      \
    int ds_buffer_size;                  \
    int ds_hit, ds_tot, ds_mis;          \
    int ds_st_hit, ds_st_tot, ds_st_mis; \
    int little

#define CACHEs_INIT(__name, __type, __csets, __cways, __cline, __unit_num, __index, __tag, __Ea, __size) \
    d_cache_##__name.getsize = __unit_num * (1 << __cline) + 128;                                        \
    d_cache_##__name.datasize = __unit_num * sizeof(__type);                                             \
    d_cache_##__name.curEaddr = 0;                                                                       \
    d_cache_##__name.tag = __tag;                                                                        \
    d_cache_##__name.accessed = 0;                                                                       \
    d_cache_##__name.miss = 0;                                                                           \
    d_cache_##__name.st_accessed = 0;                                                                    \
    d_cache_##__name.st_miss = 0;                                                                        \
    d_cache_##__name.write_back = 0;                                                                     \
    d_cache_##__name.flush = 0;                                                                          \
    d_cache_##__name.writable = 4;                                                                       \
    d_cache_##__name.modified = 0;                                                                       \
    float *ds_temp_##__name

#define CACHEs_SEC_W_RD_K(__name, __addr, __value)                                                                                         \
    ds_point_p = (char *)__addr;                                                                                                           \
    little = d_cache_##__name.curEaddr + (d_cache_##__name.getsize) - (d_cache_##__name.datasize);                                         \
    if (!((ds_point_p >= d_cache_##__name.curEaddr) && (ds_point_p <= little)))                                                            \
    {                                                                                                                                      \
        ds_EaAlign = (addrl)ds_point_p & 0xffffff80;                                                                                       \
        if (d_cache_##__name.modified)                                                                                                     \
        {                                                                                                                                  \
            spu_mfcdma32(d_cache_##__name.bbuff, d_cache_##__name.curEaddr, d_cache_##__name.getsize, d_cache_##__name.tag, MFC_PUTF_CMD); \
            d_cache_##__name.write_back++;                                                                                                 \
            d_cache_##__name.replace++;                                                                                                    \
            mfc_write_tag_mask(1 << d_cache_##__name.tag);                                                                                 \
            mfc_read_tag_status_all();                                                                                                     \
        }                                                                                                                                  \
        spu_mfcdma32(d_cache_##__name.bbuff, ds_EaAlign, d_cache_##__name.getsize, d_cache_##__name.tag, MFC_GETF_CMD);                    \
        d_cache_##__name.st_miss++;                                                                                                        \
        d_cache_##__name.modified = 0;                                                                                                     \
        mfc_write_tag_mask(1 << d_cache_##__name.tag);                                                                                     \
        mfc_read_tag_status_all();                                                                                                         \
    }                                                                                                                                      \
    else                                                                                                                                   \
    {                                                                                                                                      \
        ds_EaAlign = d_cache_##__name.curEaddr;                                                                                            \
    }                                                                                                                                      \
    d_cache_##__name.curEaddr = ds_EaAlign;                                                                                                \
    ds_temp_##__name = (((unsigned long)d_cache_##__name.bbuff + (ds_point_p - ds_EaAlign)))
#define CACHEs_SEC_W_RD(__name, __addr, __value, __type) \
    d_cache_##__name.accessed++;                         \
    CACHEs_SEC_W_RD_K(__name, __addr, __value);          \
    __value = *(__type *)(ds_temp_##__name)

#define CACHEs_SEC_W_WR(__name, __addr, __value, __type) \
    d_cache_##__name.st_accessed++;                      \
    CACHEs_SEC_W_RD_K(__name, __addr, __value);          \
    d_cache_##__name.modified = 1;                       \
    *(__type *)(ds_temp_##__name) = __value

#define CACHEs_SEC_R_RD(__name, __addr, __value, __type)                                                                \
    ds_point_p = (char *)__addr;                                                                                        \
    little = d_cache_##__name.curEaddr + (d_cache_##__name.getsize) - (d_cache_##__name.datasize);                      \
    d_cache_##__name.accessed++;                                                                                        \
    if (!((ds_point_p >= d_cache_##__name.curEaddr) && (ds_point_p <= little)))                                         \
    {                                                                                                                   \
        ds_EaAlign = (addrl)ds_point_p & 0xffffff80;                                                                    \
        spu_mfcdma32(d_cache_##__name.bbuff, ds_EaAlign, d_cache_##__name.getsize, d_cache_##__name.tag, MFC_GETF_CMD); \
        d_cache_##__name.miss++;                                                                                        \
        d_cache_##__name.replace++;                                                                                     \
        d_cache_##__name.modified = 0;                                                                                  \
        mfc_write_tag_mask(1 << d_cache_##__name.tag);                                                                  \
        mfc_read_tag_status_all();                                                                                      \
    }                                                                                                                   \
    else                                                                                                                \
    {                                                                                                                   \
        ds_EaAlign = d_cache_##__name.curEaddr;                                                                         \
    }                                                                                                                   \
    d_cache_##__name.curEaddr = ds_EaAlign;                                                                             \
    ds_temp_##__name = (((unsigned long)d_cache_##__name.bbuff + (ds_point_p - ds_EaAlign)));                           \
    __value = *(__type *)(ds_temp_##__name)

#define CACHEs_FLUSH_K(__name)                                                                                                     \
    spu_mfcdma32(d_cache_##__name.bbuff, d_cache_##__name.curEaddr, d_cache_##__name.getsize, d_cache_##__name.tag, MFC_PUTF_CMD); \
    d_cache_##__name.flush++;                                                                                                      \
    mfc_write_tag_mask(1 << d_cache_##__name.tag);                                                                                 \
    mfc_read_tag_status_all()
#define CACHEs_FLUSH(__name, __Ea, __size) \
    CACHEs_FLUSH_K(__name)

#define CACHEs_STATS(__name)                                                                                    \
    ds_buffer_size = d_cache_##__name.getsize;                                                                  \
    ds_tot = d_cache_##__name.accessed;                                                                         \
    ds_mis = d_cache_##__name.miss;                                                                             \
    ds_hit = ds_tot - ds_mis;                                                                                   \
    ds_statsf = (double)((float)ds_hit / (float)ds_tot);                                                        \
    printf("%10.5f \n", ds_statsf);                                                                             \
    ds_st_tot = d_cache_##__name.st_accessed;                                                                   \
    ds_st_mis = d_cache_##__name.st_miss;                                                                       \
    ds_st_hit = ds_tot - ds_mis;                                                                                \
    ds_st_statsf = ds_st_hit / ds_st_tot;                                                                       \
    printf("name=db_diag type=READ_WRITE nway=1 nsets=1 linesz=%d totsz=%d\n", ds_buffer_size, ds_buffer_size); \
    printf("          READ                      WRITE\n");                                                      \
    printf("           --------------------      --------------------\n");                                      \
    printf("           SET      HIT     MISS  HITPCT      HIT     MISS  HITPCT     REPL     WRBK FLUSH\n");     \
    printf("           0    hit=%lu        mis=%lu  tot=%lu ratio=%10.5f     hit=%lu    mis=%lu  ratio=%10.5f   repl= %lu      wrbk=%lu  flush=%d  \n", ds_hit, ds_mis, ds_tot, ds_statsf, ds_st_hit, ds_st_mis, ds_st_statsf, d_cache_##__name.replace, d_cache_##__name.write_back, d_cache_##__name.flush)