#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "xapp058/micro.h"
#include "my_spi.h"

/* Upload und Flash XSVF */
static esp_err_t upload_flash_xsvf_post_handler(httpd_req_t *req)
{
    uint8_t * data = heap_caps_malloc(req->content_len, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM);
    uint32_t offset = 0;
    if (!data) 
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,"Datei zu gross!");
        return ESP_FAIL;
    }

    while (offset<req->content_len)
    {
        int received = httpd_req_recv(req, (void*)data+offset, req->content_len-offset);

        if (received == HTTPD_SOCK_ERR_TIMEOUT) 
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,"Upload timeout");
            return ESP_FAIL;
        }
        offset += received;
    }

    uint8_t r = xsvfExecute(data, req->content_len);

    if (r==0)
    {
        httpd_resp_sendstr(req, "XSVF Flash erfolgreich.\n");
    }
    else
    {
        httpd_resp_sendstr(req, "XSVF Flash Fehlgeschlagen!\n");
        printf("XSVF Flash Fehler %d\n",r);
    }

    free(data);
    return ESP_OK;
}

static esp_err_t uploader_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><head></head><body>");
    httpd_resp_sendstr_chunk(req, "<h2>XSVF Upload und Flash</h2>");
    httpd_resp_sendstr_chunk(req, "<input id=\"newfile\" type=\"file\" >");
    httpd_resp_sendstr_chunk(req, "<button id=\"upload\" type=\"button\" onclick=\"upload()\">Upload</button>\r\n");
    httpd_resp_sendstr_chunk(req, "<script>\r\nfunction upload() {\r\n");
    httpd_resp_sendstr_chunk(req, "var fileInput = document.getElementById(\"newfile\").files;\r\ndocument.getElementById(\"newfile\").disabled = true;\r\n");
    httpd_resp_sendstr_chunk(req, "document.getElementById(\"upload\").disabled = true;\r\nvar file = fileInput[0];\r\n");
    httpd_resp_sendstr_chunk(req, "var xhttp = new XMLHttpRequest();\r\nxhttp.onreadystatechange = function() {\r\nif (xhttp.readyState == 4) {\r\n");
    httpd_resp_sendstr_chunk(req, "if (xhttp.status == 200) {\r\ndocument.open();\r\ndocument.write(xhttp.responseText);\r\ndocument.close();\r\n");
    httpd_resp_sendstr_chunk(req, "} else if (xhttp.status == 0) {alert(\"Server closed the connection abruptly!\");location.reload()\r\n");
    httpd_resp_sendstr_chunk(req, "} else {alert(xhttp.status + \" Error!\\n\" + xhttp.responseText);\r\nlocation.reload()\r\n}\r\n}\r\n};\r\n");
    httpd_resp_sendstr_chunk(req, "xhttp.open(\"POST\", \"/xapp058/upload_flash\", true);\r\nxhttp.send(file);\r\n}\r\n</script>");
    httpd_resp_sendstr_chunk(req, "</body></html>");
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

static esp_err_t spi_test_get_handler(httpd_req_t *req)
{
	get_registers();
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><head></head><body>");
    httpd_resp_sendstr_chunk(req, "<h2>SPI Transfer und Register Test</h2>");
	httpd_resp_sendstr_chunk(req, "<form action=\"/spi_test.html\" method=\"post\"><table border=\"0\" ><tr>");
    char b[128];
	for (int i=0x400;i<0x410;i++)
	{
		snprintf(b,128,"<td>%3X</td>",i);
		httpd_resp_sendstr_chunk(req, b);
	}
	httpd_resp_sendstr_chunk(req, "<td>&nbsp;</td></tr><tr>");
	for (int i=0;i<16;i++)
	{
		snprintf(b,128,"<td><input style=\"width:30px\" type=\"text\" maxlength=\"2\" value=\"%02X\" id=\"reg%d\" name=\"reg%d\"></td>",regs[i],i,i);	
		httpd_resp_sendstr_chunk(req, b);
	}
	httpd_resp_sendstr_chunk(req, "<td><button type=\"submit\">Setzen</button></td></tr></table></form></body></html>");
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

static esp_err_t spi_test_post_handler(httpd_req_t *req)
{
	char a[8];
	char b[8];
	int c;
    char d[256];
    c = httpd_req_recv(req, d, 256);
    d[c] = 0;
	for (int i=0;i<16;i++)
	{
		snprintf(a,8,"reg%d",i);
		httpd_query_key_value(d,a,b,8);
		c = 0;
		sscanf(b, "%x", &c);
		regs[i] = c;
	}
	set_registers();
	return spi_test_get_handler(req);
}

void start_webserver()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) != ESP_OK) 
    {
        printf("Failed to start file server!");
        return;
    }

    httpd_uri_t uploader_get_page = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = uploader_get_handler
    };
    httpd_register_uri_handler(server, &uploader_get_page);

    httpd_uri_t spitest_get_page = {
        .uri       = "/spi_test.html",
        .method    = HTTP_GET,
        .handler   = spi_test_get_handler
    };
    httpd_register_uri_handler(server, &spitest_get_page);
    
    httpd_uri_t spitest_post_page = {
        .uri       = "/spi_test.html",
        .method    = HTTP_POST,
        .handler   = spi_test_post_handler
    };
    httpd_register_uri_handler(server, &spitest_post_page);
    
    httpd_uri_t upload_flash_xsvf_post_page = {
        .uri       = "/xapp058/upload_flash",
        .method    = HTTP_POST,
        .handler   = upload_flash_xsvf_post_handler
    };
    httpd_register_uri_handler(server, &upload_flash_xsvf_post_page);
}

void stop_webserver()
{

}