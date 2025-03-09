project('downloader', 'c')
sources = ['src/main.c', 'src/download.c', 'src/http_header.c']
dep_openssl = dependency('openssl', method: 'cmake')
executable('downloader', sources, dependencies: [dep_openssl])
