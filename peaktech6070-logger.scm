;;; Use like this:
;;;
;;; > csi -s peaktech6070-logger.scm /dev/ttyUSB0
;;; OFF	     0mV      0mA 	 ;;  12000mV    300mA
;;; OFF	     0mV      0mA 	 ;;  12000mV    300mA
;;; OFF	     0mV      0mA 	 ;;  12000mV    300mA
;;; ON 	 12000mV      4mA 	 ;;  12000mV    300mA
;;; ON 	 12000mV      0mA 	 ;;  12000mV    300mA

(include "peaktech6070.scm")
(import chicken.process-context)

(when (null? (command-line-arguments))
  (error "usage: <tty>
   e.g. /dev/ttyUSB0"))

(define path (car (command-line-arguments)))
(define fd (file-open path (+ open/binary open/rdwr)))
(current-input-port (open-input-file* fd))

(define (frame-print-all T on? mVc mAc mVt mAt)
  (fmt #t
       (if on? "ON " "OFF") "\t"
       (pad/left 6 mVc) "mV "
       (pad/left 6 mAc) "mA " "\t ;; "
       (pad/left 6 mVt) "mV "
       (pad/left 6 mAt) "mA" 
       nl))

(let loop ()
  (get-current* fd)
  (thread-sleep! 0.7) ;; the bundled "Power Management System" seems to poll at 1s intervals
  (let ((frame (read-frame)))
    (cond (frame
           (with-input-from-string frame
             (lambda ()
               (call-with-frame frame-print-all))))
          (else (print ".")))
    (loop)))
