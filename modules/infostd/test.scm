(dynamic-load "./infostd-stemmer.so")
(define st (stemmer-read "english.stemmer"))
(stemmer-root st "tests")
(stemmer-forms st "tests")
