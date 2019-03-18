#include "modbus_tcp_test.h"
#include "modbus.h"
#include "stdio.h"
#include "string.h"
#include <dfs_posix.h>

static void test_thread(void *param)
{
    rt_thread_mdelay(5000);
    uint16_t tab_reg[64] = {0};
    modbus_t *ctx = RT_NULL;
    ctx = modbus_new_tcp("192.168.1.138",601);

    modbus_set_slave(ctx, 3);
    modbus_connect(ctx);
    modbus_set_response_timeout(ctx, 0, 1000000);
    int num = 0;
    while (1)
    {
        memset(tab_reg, 0, 64 * 2);
        int regs = modbus_read_registers(ctx, 0, 20, tab_reg);
        printf("-------------------------------------------\n");
        printf("[%4d][read num = %d]", num, regs);
        num++;
        int i;
        for (i = 0; i < 20; i++)
        {
            printf("<%#x>", tab_reg[i]);
        }
        printf("\n");
        printf("-------------------------------------------\n");
        rt_thread_mdelay(5000);
    }
    //7-关闭modbus端口
    modbus_close(ctx);

    //8-释放modbus资源
    modbus_free(ctx);
}

//static void test_thread(void *param)
//{
//    rt_thread_mdelay(5000);
//    int s = -1;
//    modbus_t *ctx = NULL;
//    modbus_mapping_t *mb_mapping = NULL;
//    int rc;
//    
//    ctx = modbus_new_tcp(RT_NULL, 1502);
//    s = modbus_tcp_listen(ctx, 1);
//    modbus_tcp_accept(ctx, &s);
//    mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
//                                    MODBUS_MAX_READ_REGISTERS, 0);
//    while(1)
//    {
//        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

//        rc = modbus_receive(ctx, query);
//        if (rc > 0) {
//            modbus_reply(ctx, query, rc, mb_mapping);
//        } else if (rc  == -1) {
//            /* Connection closed by the client or error */
//            break;
//        }
//    }
//    modbus_mapping_free(mb_mapping);
//    if (s != -1) {
//        close(s);
//    }
//    /* For RTU, skipped by TCP (no TCP connect) */
//    modbus_close(ctx);
//    modbus_free(ctx);
//}

static int tcp_test_init(void)
{
    rt_thread_t tid;
    tid = rt_thread_create("test",
                           test_thread, RT_NULL,
                           2048,
                           12, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(tcp_test_init);
