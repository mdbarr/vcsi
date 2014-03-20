#!/usr/local/bin/vcsi
; Load the module
(dynamic-load "/home/fm/c-progs/vcsi/modules/toc2/toc2.so")

(display "Connecting...\n")

; Create the toc object
(define tocc (make-toc "kn0wbot" "35707249v"))

; Connect
(toc-connect tocc)

(display "Connected!\n")

; Basic Handlers
(define (toc-on-im screename message auto)
  (if auto
      (display (format ">~a< ~a\n" screename message))
      (display (format "<~a> ~a\n" screename message)))
  ; Respond
  (toc-im tocc screename "shutup"))


(define (toc-on-update-buddy screename online evil signon idle away)
  (display (format "* ~a: ~a ~a ~a ~a ~a\n" 
		   screename online evil signon idle away)))

(define (toc-on-error text) 
  (display (format "ERROR: ~a\n" text)))

; Add the handlers

(toc-handler-set! tocc 'im toc-on-im)
(toc-handler-set! tocc 'update-buddy toc-on-update-buddy)
(toc-handler-set! tocc 'error toc-on-error)

; Start the handler
(define (toc-handler) (toc-handle-one-event tocc) (toc-handler))
(toc-handler)

; Done
#t
