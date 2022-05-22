(include "peaktech6070.scm")
(import test)

(test-group
 "ser deser"
 (test "\x00\x01" (u16->string 1))
 (test "\x00\x10" (u16->string #x10))
 (test "\x01\x00" (u16->string #x100))
 (test "\xff\xff" (u16->string #xffff))

 (test #x0102 (with-input-from-string "\x01\x02" read-u16)))


(define pack
  (conc "\xf7\x01\x03\x04\x05"
        "\x61"
        "\x00\x00"
        "\x64\x00" "\x00\x00\x64\x01\x2c\x24\xd7\xfd"))

(define pack ;; 0.3V
  (conc "\xf7\x01\x03\x04\x05"
        "\x61\x00" ;; on/off, ?
        "\x00\x1e" ;; cV
        "\x00\x00" ;; mA
        "\x00\x1e" ;; cV (readback)
        "\x0d\x80" ;; mA (readback)
        "\xdb\xb4\xfd" ;; checksum
        ))

