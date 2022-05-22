(import chicken.string chicken.blob chicken.bitwise chicken.file.posix
        chicken.port chicken.io
        srfi-18 fmt)

(define (u16->string n)
  (conc (integer->char (bitwise-and #xFF (arithmetic-shift n -8)))
        (integer->char (bitwise-and #xFF (arithmetic-shift n 0)))))

(define I
  (getter-with-setter
   (lambda ()
     (error "TODO"))
   (lambda (mA) ;;            ,-- CH?     
     (conc "\xf7010a0a" "0x01" (u16->string mA)))))

(conc (string->blob "hi"))

(define (expect expected where)
  (let ((actual (read-byte)))
    (unless (eq? actual expected) (error (conc "expected mismatch " where) expected actual))
    #t))

(define (read-u16)
  (let* ((n0 (read-byte))
         (n1 (read-byte)))
    (+ (* 256 n0) n1)))

(define (call-with-frame proc)
  (expect #xf7 "header")
  (expect #x01 "h1")
  (expect #x03 "h2")
  (expect #x04 "h3")
  (expect #x05 "h4")
  (define on/off (= #x61 (read-byte))) ;; output on/off
  (read-byte) ;; (expect 0 "?")

  (define mVc (* 10 (read-u16))) ;; nobody wants centiVolts I think
  (define mAc (*  1 (read-u16)))

  (define mVt (* 10 (read-u16)))
  (define mAt (*  1 (read-u16)))

  (read-u16) ;; (read-some-checksum16)
  (expect #xfd "frame end marker")

  (proc 'current on/off mVc mAc mVt mAt))

(define (frame-current-mA) (call-with-frame (lambda (T on? mVc mAc mVt mAt) mAc)))
(define (frame-current-mV) (call-with-frame (lambda (T on? mVc mAc mVt mAt) mVc)))
(define (frame-target-mA)  (call-with-frame (lambda (T on? mVc mAc mVt mAt) mAt)))
(define (frame-target-mV)  (call-with-frame (lambda (T on? mVc mAc mVt mAt) mVt)))

(define (get-current* fd)
  (file-write fd "\xf7\x01\x03\x04\x05\xe2\xea\xfd"))

(define (read-frame)
  (let loop ((packet ""))
    (let ((got (read-char)))
      (if (eof-object? got)
          #f
          (if (eq? (string-ref "\xfd" 0) got)
              (conc packet got)
              (loop (conc packet got)))))))



