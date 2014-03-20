#!/usr/local/bin/vcsi
(dynamic-load "/home/fm/c-progs/vcsi/modules/http/http.so")
(http-simple-get "http://sassmaster.com/index.html")
(http-simple-get "http://sassmaster.com/")
(http-simple-get "http://sassmaster.com")

