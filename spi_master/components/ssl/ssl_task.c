/*
 * https_task.c
 *
 *  Created on: 2018年3月12日
 *      Author: MO
 */
/*
 * ssl.c
 *
 *  Created on: 2018年3月11日
 *      Author: MO
 */
#include "ssl_task.h"

#include <stddef.h>
#include "openssl/ssl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/api.h"

#define OPENSSL_DEMO_THREAD_NAME "ssl_demo"
#define OPENSSL_DEMO_THREAD_STACK_WORDS 4096
#define OPENSSL_DEMO_THREAD_PRORIOTY 6

#define OPENSSL_DEMO_FRAGMENT_SIZE 8192

#define OPENSSL_DEMO_static_TCP_PORT 1000

//#define OPENSSL_DEMO_TARGET_NAME "iot.espressif.cn"
////#define OPENSSL_DEMO_TARGET_NAME "www.baidu.com"
//#define OPENSSL_DEMO_TARGET_TCP_PORT 443
//
////#define OPENSSL_DEMO_REQUEST "{\"path\": \"/v1/ping/\", \"method\": \"GET\"}\r\n"
//
//#define OPENSSL_DEMO_REQUEST "/ HTTP/1.1 Host:iot.espressif.cn:443"


#define OPENSSL_DEMO_TARGET_NAME "free-api.heweather.com"

#define OPENSSL_DEMO_TARGET_TCP_PORT 443

#define OPENSSL_DEMO_REQUEST "GET /s6/weather/now?location=shanghai&key=dea2f74f82314806b66ce788fb7120e5 HTTP/1.1\r\nHost: free-api.heweather.com\r\nCache-Control: no-cache\r\nconnection: close\r\n\r\n"

#define OPENSSL_DEMO_RECV_BUF_LEN 1024

static xTaskHandle openssl_handle;

static char send_data[] = OPENSSL_DEMO_REQUEST;
static int send_bytes = sizeof(send_data);

//static char recv_buf[OPENSSL_DEMO_RECV_BUF_LEN];

static void openssl_demo_thread(void *p)
{
    int ret;

    SSL_CTX *ctx;
    SSL *ssl;

    int socket;
    struct sockaddr_in sock_addr;

    ip_addr_t target_ip;

    int recv_bytes = 0;
    char *recv_buf = malloc(OPENSSL_DEMO_RECV_BUF_LEN);
    vTaskDelay(5000/ portTICK_PERIOD_MS);

    printf("OpenSSL demo thread start...\n");

    do {
        ret = netconn_gethostbyname(OPENSSL_DEMO_TARGET_NAME, &target_ip);
    } while(ret);
    printf("get target IP is %d.%d.%d.%d\n", (unsigned char)((target_ip.u_addr.ip4.addr & 0x000000ff) >> 0),
                                                (unsigned char)((target_ip.u_addr.ip4.addr & 0x0000ff00) >> 8),
                                                (unsigned char)((target_ip.u_addr.ip4.addr & 0x00ff0000) >> 16),
                                                (unsigned char)((target_ip.u_addr.ip4.addr & 0xff000000) >> 24));

    printf("create SSL context ......");
    ctx = SSL_CTX_new(TLSv1_1_client_method());
    if (!ctx) {
        printf("failed\n");
        goto failed1;
    }
    printf("OK\n");

    printf("set SSL context read buffer size ......");
    SSL_CTX_set_default_read_buffer_len(ctx, OPENSSL_DEMO_FRAGMENT_SIZE);
    ret = 0;
    if (ret) {
        printf("failed, return %d\n", ret);
        goto failed2;
    }
    printf("OK\n");

    printf("create socket ......");
    socket = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        printf("failed\n");
        goto failed3;
    }
    printf("OK\n");

    printf("bind socket ......");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = 0;
    sock_addr.sin_port = htons(OPENSSL_DEMO_static_TCP_PORT);
    ret = bind(socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if (ret) {
        printf("failed\n");
        goto failed4;
    }
    printf("OK\n");

    printf("socket connect to remote ......");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = target_ip.u_addr.ip4.addr;
    sock_addr.sin_port = htons(OPENSSL_DEMO_TARGET_TCP_PORT);
    ret = connect(socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if (ret) {
        printf("failed OPENSSL_DEMO_TARGET_NAME\n");
        goto failed5;
    }
    printf("OK\n");

    printf("create SSL ......");
    ssl = SSL_new(ctx);
    if (!ssl) {
        printf("failed\n");
        goto failed6;
    }
    printf("OK\n");

    SSL_set_fd(ssl, socket);

    printf("SSL connected to %s port %d ......", OPENSSL_DEMO_TARGET_NAME, OPENSSL_DEMO_TARGET_TCP_PORT);
    ret = SSL_connect(ssl);
    if (!ret) {
        printf("failed, return [-0x%x]\n", -ret);
        goto failed7;
    }
    printf("OK\n");

    printf("send request to %s port %d ......", OPENSSL_DEMO_TARGET_NAME, OPENSSL_DEMO_TARGET_TCP_PORT);
    ret = SSL_write(ssl, send_data, send_bytes);
    if (ret <= 0) {
        printf("failed, return [-0x%x]\n", -ret);
        goto failed8;
    }
    printf("OK\n\n");

    do {
        ret = SSL_read(ssl, recv_buf, OPENSSL_DEMO_RECV_BUF_LEN - 1);
        if (ret <= 0) {
            break;
        }
        recv_bytes += ret;
        printf("%s", recv_buf);
    } while (1);
    printf("read %d bytes data from %s ......\n", recv_bytes, OPENSSL_DEMO_TARGET_NAME);

failed8:
    SSL_shutdown(ssl);
failed7:
    SSL_free(ssl);
failed6:
failed5:
failed4:
    close(socket);
failed3:
failed2:
    SSL_CTX_free(ctx);
failed1:
    vTaskDelete(NULL);

    printf("task exit\n");

    return ;
}

void user_conn_init(void)
{
    int ret;
    vTaskDelay(4000 / portTICK_PERIOD_MS);
    ret = xTaskCreate(openssl_demo_thread,
                      OPENSSL_DEMO_THREAD_NAME,
                      OPENSSL_DEMO_THREAD_STACK_WORDS,
                      NULL,
                      OPENSSL_DEMO_THREAD_PRORIOTY,
                      &openssl_handle);
    if (ret != pdPASS)  {
        printf("create thread %s failed\n", OPENSSL_DEMO_THREAD_NAME);
        return ;
    }
}









