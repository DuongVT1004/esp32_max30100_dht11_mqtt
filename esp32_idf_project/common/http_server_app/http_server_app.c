
/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "http_server_app.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
//#include <nvs_flash.h>
#include <sys/param.h>
//#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
//#include "protocol_examples_common.h"

#include <esp_http_server.h>
#include <string.h>

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");


static const char *TAG = "HTTP_SERVER";
static httpd_handle_t server = NULL;
static httpd_req_t *REG = NULL;

static http_post_callback_t http_post_wifi_callback = NULL;


/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

    return ESP_OK;
}

static const httpd_uri_t get_data_web = {
    .uri       = "/getweb",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};


static esp_err_t wifi_handler(httpd_req_t *req)
{
    char buf[100];
    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    http_post_wifi_callback(buf, req->content_len);
    // printf("DATA: %s\n", buf);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t wifi_post = {
    .uri       = "/wifiinfo",
    .method    = HTTP_POST,
    .handler   = wifi_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/dht11", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/dht11 URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } //else if (strcmp("/echo", req->uri) == 0) {
    //     httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
    //     /* Return ESP_FAIL to close underlying socket */
    //     return ESP_FAIL;
    // }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

void start_webserver(void)
{
    //httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &get_data_web);
        httpd_register_uri_handler(server, &wifi_post);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        //httpd_register_uri_handler(server, &ctrl);
        //return server;
    }
    else{
        ESP_LOGI(TAG, "Error starting server!");
    }
    //return NULL;
}

void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}

void http_set_callback_wifi(void *cb)
{
    http_post_wifi_callback = cb;
}
