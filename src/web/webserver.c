#include "esp_http_server.h"
#include "esp_log.h"

#define TAG "webserver"

esp_err_t base_path_get_handler(httpd_req_t *req)
{
    /* test response */
    const char resp[] =
        "<html>"
        "<h1 style>ESP32 Server</h1>"
        "<button onclick=\"alert('works')\">Test</button>"
        "<div id=\"counter\">0</div>"
        "<button id=\"dec\">Dec</button>"
        "<button id=\"inc\">Inc</button>"
        "<img src='https://i.scdn.co/image/ab67616d00001e024021df765e3ab3a966dfaaec' />"
        "</html>"
        "<script>"
        "let count = 0;"
        "const countEl = document.getElementById(\"counter\");"
        "const rerender = () => { if (countEl) countEl.innerText = String(count); };"
        "document.getElementById(\"inc\")?.addEventListener(\"click\", () => { count++; rerender(); });"
        "document.getElementById(\"dec\")?.addEventListener(\"click\", () => { count--; rerender(); });"
        "</script>";
    return httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
}

httpd_uri_t base_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = base_path_get_handler,
    .user_ctx = NULL};

httpd_handle_t start_webserver(void)
{
    // create default configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // empty handle to esp_http_server
    httpd_handle_t server = NULL;

    // start the httpd server
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // register URI handlers
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &base_get));
    }

    // if server failed to start, handle will be NULL */
    return server;
}