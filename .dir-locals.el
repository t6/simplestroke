;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

((nil
  (eval
   (lambda ()
     (let ((args (progn
                   (shell-cd (projectile-project-root))
                   (split-string (shell-command-to-string
                                  "make company-clang-arguments")
                                 "\n"))))
       (setq-local company-clang-arguments args)
       (setq-local flycheck-clang-args args))))))
