# downloader

## for debugging

use meson setup --wipe build -Db_sanitize=SANITIZER_NAME

### test links

google.com
kernel.org
https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.13.6.tar.xz

## TODO

- [ ] Finish chunked transfer with http 1.1 (https://stackoverflow.com/questions/7232931/receiving-chunked-http-data-with-winsock)
- [ ] Add a way to extend the downloader with plugins (shared libraries in a specific folder) that would add protocols (ex : ftp, etc)