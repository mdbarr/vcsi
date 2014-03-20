; Load the module
(dynamic-load "/home/barr/c-progs/vcsi/modules/irc/irc.so")

(display "Connecting...\n")

; Setup connection variables
(define irc-server "irc.sassmaster.com")
(define irc-port 6667)
(define irc-mynick "vcsi")
(define irc-name "VCSI Scheme Interpreter")
(define irc-channel "#vcsi")

; Create the irc object 
(define ircc (make-irc irc-server irc-port irc-mynick irc-name))

; Create simple handlers
(define (irc-public-handler channel target source hostmask text)
  (display "<") (display source) (display "> ") (display text) (newline))
  
(define (irc-msg-handler channel target source hostmask text)
  (display ">") (display source) (display "< ") (display text) (newline))

(define (irc-action-handler channel target source hostmask text)
  (display "*") (display source) (display " ") (display text) (newline))

(define (irc-kick-handler channel target source hostmask text)
  (if (eq? target irc-mynick)
      (begin (irc-join ircc channel) (irc-privmsg ircc channel "buh!"))))

(define (irc-topic-handler channel target source hostmask topic)
  (display "Topic: ") (display topic) (newline))

; Set the handlers
(irc-handler-set! ircc 'public irc-public-handler)

(irc-handler-set! ircc 'msg irc-msg-handler)

(irc-handler-set! ircc 'action irc-action-handler)

(irc-handler-set! ircc 'kick irc-kick-handler)

(irc-handler-set! ircc 'topic irc-topic-handler)

(define (irc-handler) (irc-handle-one-event ircc) (irc-handler))

; Connect
(irc-connect ircc)

(display "Joining channel irc-channel\n")

; Join the channel
(irc-join ircc irc-channel)

; Greet
(irc-privmsg ircc irc-channel "hello")

(display "Starting handler.\n")

; Start the handler in a thread
(define irc-thread (call-with-new-thread (irc-handler)))

; Done
#t

