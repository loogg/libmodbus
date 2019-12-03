#include "modbus_tcp_test.h"
#include <modbus.h>
#include <stdio.h>
#include <string.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <sal_socket.h>

// static void test_thread(void *param)
// {
//     uint16_t tab_reg[64] = {0};
//     modbus_t *ctx = RT_NULL;
    
//     ctx = modbus_new_tcp("192.168.1.103", 601, AF_INET);
//     modbus_set_slave(ctx, 3);
//     modbus_set_response_timeout(ctx, 0, 1000000);
// _mbtcp_start:
//     if(modbus_connect(ctx) < 0)
//         goto _mbtcp_restart;
    
//     int num = 0;
//     while (1)
//     {
//         memset(tab_reg, 0, 64 * 2);
//         int regs = modbus_read_registers(ctx, 0, 20, tab_reg);
//         if(regs < 0)
//             goto _mbtcp_restart;
//         printf("-------------------------------------------\n");
//         printf("[%4d][read num = %d]", num, regs);
//         num++;
//         int i;
//         for (i = 0; i < 20; i++)
//         {
//             printf("<%#x>", tab_reg[i]);
//         }
//         printf("\n");
//         printf("-------------------------------------------\n");
//         rt_thread_mdelay(1000);
//     }

// _mbtcp_restart:
//     //7-关闭modbus端口
//     modbus_close(ctx);
//     rt_thread_mdelay(2000);
//     goto _mbtcp_start;
    
//     //8-释放modbus资源
//     modbus_free(ctx);
// }

#define MAX_CLIENT_NUM  3
#define CLIENT_TIMEOUT  10      //单位 s

typedef struct
{
    int fd;
    rt_tick_t tick_timeout;
}client_session_t;

static void test_thread(void *param)
{
    int server_fd = -1;
    modbus_t *ctx = NULL;
    modbus_mapping_t *mb_mapping = NULL;
    client_session_t client_session[MAX_CLIENT_NUM];

    for (int i = 0; i < MAX_CLIENT_NUM; i++)
    {
        client_session[i].fd = -1;
        client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
    }

    int max_fd = -1;
    fd_set readset;
    int rc;
    struct timeval select_timeout;
    select_timeout.tv_sec = 1;
    select_timeout.tv_usec = 0;

    ctx = modbus_new_tcp(RT_NULL, 1502, AF_INET);
    RT_ASSERT(ctx != RT_NULL);
    mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);
    RT_ASSERT(mb_mapping != RT_NULL);

_mbtcp_start:
    server_fd = modbus_tcp_listen(ctx, 1);
    if (server_fd < 0)
        goto _mbtcp_restart;

    while (1)
    {
        max_fd = -1;
        FD_ZERO(&readset);
        FD_SET(server_fd, &readset);

        if(max_fd < server_fd)
            max_fd = server_fd;
        
        for (int i = 0; i < MAX_CLIENT_NUM; i++)
        {
            if(client_session[i].fd >= 0)
            {
                FD_SET(client_session[i].fd, &readset);
                if(max_fd < client_session[i].fd)
                    max_fd = client_session[i].fd;
            }
        }

        rc = select(max_fd + 1, &readset, RT_NULL, RT_NULL, &select_timeout);
        if(rc < 0)
        {
            goto _mbtcp_restart;
        }
        else if(rc > 0)
        {
            if(FD_ISSET(server_fd, &readset))
            {
                int client_sock_fd = modbus_tcp_accept(ctx, &server_fd);
                if(client_sock_fd >= 0)
                {
                    int index = -1;
                    for (int i = 0; i < MAX_CLIENT_NUM; i++)
                    {
                        if(client_session[i].fd < 0)
                        {
                            index = i;
                            break;
                        }
                    }
                    if(index >= 0)
                    {
                        client_session[index].fd = client_sock_fd;
                        client_session[index].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
                    }
                    else
                    {
                        close(client_sock_fd);
                    }
                }
            }

            for (int i = 0; i < MAX_CLIENT_NUM; i++)
            {
                if(client_session[i].fd >= 0)
                {
                    if(FD_ISSET(client_session[i].fd, &readset))
                    {
                        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
                        modbus_set_socket(ctx, client_session[i].fd);

                        rc = modbus_receive(ctx, query);
                        if (rc > 0)
                        {
                            rc = modbus_reply(ctx, query, rc, mb_mapping);
                            if(rc < 0)
                            {
                                close(client_session[i].fd);
                                client_session[i].fd = -1;
                            }
                            else
                            {
                                client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
                            }
                        }
                        else
                        {
                            close(client_session[i].fd);
                            client_session[i].fd = -1;
                        }
                    }
                }
            }
        }

        // 客户端超时未收到数据断开
        for(int i =0;i<MAX_CLIENT_NUM;i++)
        {
            if(client_session[i].fd >= 0)
            {
                //超时
                if((rt_tick_get() - client_session[i].tick_timeout) < (RT_TICK_MAX / 2))
                {
                    close(client_session[i].fd);
                    client_session[i].fd = -1;
                }
            }
        }
    }

_mbtcp_restart:
    if(server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }

    for(int i =0;i<MAX_CLIENT_NUM;i++)
    {
        if(client_session[i].fd >= 0)
        {
            close(client_session[i].fd);
            client_session[i].fd = -1;
        }
    }

    rt_thread_mdelay(5000);
    goto _mbtcp_start;

    modbus_free(ctx);
}

static int tcp_test_init(void)
{
    rt_thread_t tid;
    tid = rt_thread_create("mbtcp_test",
                           test_thread, RT_NULL,
                           2048,
                           12, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(tcp_test_init);
