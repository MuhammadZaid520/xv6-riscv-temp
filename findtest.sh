mkdir a
echo > a/b
mkdir a/aa
echo > a/aa/b
echo DONE
find . b -exec echo hello
