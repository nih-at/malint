EXTRA_DIST=runtest ${TESTCASES} \
	frame-l3crc.in frame-l3crc.log \
	frame-long.in frame-long.log \
	frame-short.in frame-short.log \
	id3v1-short.in id3v1-short.log \
	id3v1-noseek.log \
	id3v1-end.in id3v1-end.log

TESTCASES=frame-l3crc.test frame-long.test frame-short.test \
	id3v1-end.test id3v1-noseek.test id3v1-short.test

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
