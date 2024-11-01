#/bin/sh

echo "Generating makefiles using premake ==========="
cd ..
premake5 gmake
cd scripts

echo "Building gbemu-tests ==========="
make -C ../build gbemu-tests config=coverage -j8

echo "Running gbemu-tests ==========="
../build/bin/coverage/gbemu-tests

echo "Generating lcov report ==========="
cd ../scripts
lcov --capture --directory ../build/obj/coverage --output-file gbemu-tests.info
# remove stuff that is not supposed to be tested
lcov --remove gbemu-tests.info "/usr*" --output-file gbemu-tests.info
lcov --remove gbemu-tests.info "third-party*" --output-file gbemu-tests.info
lcov --remove gbemu-tests.info "doctest*" --output-file gbemu-tests.info
lcov --remove gbemu-tests.info "tests*" --output-file gbemu-tests.info
lcov --remove gbemu-tests.info "gbdebug*" --output-file gbemu-tests.info

echo "Generating html report ==========="
genhtml -o ./html -t "gbemu-tests coverage report" gbemu-tests.info
