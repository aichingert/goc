#include "dsl/dsl.h"
#include "wcl/wcl.h"

#include <stddef.h>

int main(void) {
    dsl_compile();

    char *url = "stake.com";
    request_site(url);

    return 0;
}
