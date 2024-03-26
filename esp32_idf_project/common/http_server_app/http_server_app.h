#ifndef _HTTP_SERVER_APP_H_
#define _HTTP_SERVER_APP_H_

#include <stdint.h>

typedef void (*http_post_callback_t) (char* data, uint16_t len);
// typedef void (*http_get_callback_t) (void);
// typedef void (*http_get_data_callback_t) (char* data, uint16_t len);

void start_webserver(void);
void stop_webserver(void);
void http_set_callback_wifi(void *cb);


#endif