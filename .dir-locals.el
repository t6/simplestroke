;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

((nil
  (eval
   (lambda ()
     (setq-local company-clang-arguments
                 (progn
                   (shell-cd (projectile-project-root))
                   (split-string (shell-command-to-string
                                  "make company-clang-arguments")
                                 "\n")))))))
