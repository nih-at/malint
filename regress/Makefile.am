EXTRA_DIST=runtest ${TESTCASES} \
	id3v1-short.in

TESTCASES=id3v1-short.test

check:
	@failed=0; all=0; \
	srcdir=$(srcdir); export srcdir; \
	for tst in ${TESTCASES}; do \
	  t=`basename $$tst .test`; \
	  if ${TESTS_ENVIRONMENT} $(srcdir)/runtest $$t; then \
	    all=`expr $$all + 1`; \
	    echo "PASS: $$tst"; \
	  elif test $$? -ne 77; then \
	    all=`expr $$all + 1`; \
	    failed=`expr $$failed + 1`; \
	    echo "FAIL: $$tst"; \
	  fi; \
	done; \
	if test "$$failed" -eq 0; then \
	  banner="All $$all tests passed"; \
	else \
	  banner="$$failed of $$all tests failed"; \
	fi; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"; \
	test "$$failed" -eq 0