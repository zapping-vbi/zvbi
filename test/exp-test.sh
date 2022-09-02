#!/bin/bash

sliced_bz2="$1"
pgno=${2-100} # 100 ... 899 or empty for all pages, default 100

if [ "$3" = "--val" ] ; then
  val="valgrind --leak-check=full --error-exitcode=1"
fi

die() {
  echo $1
  exit 1
}

target_loop() {
  local option="$1"

  for target in 1 2 3 5 ; do
    # ttxfilter to clean up malformed streams.
    bunzip2 < "$sliced_bz2" | ./ttxfilter 100-899 \
      | $val ./export -a $target $module$option $pgno \
        -o exp-test-out-$target$option.$module \
      || die
    for file in exp-test-out-1$option*.$module ; do
      file2=`echo "$file" | sed "s/^exp-test-out-1/exp-test-out-$target/"`
      cmp $file $file2 \
        || die "Mismatch btw $module$option target 1 and $target ($file2)"
      echo "$module$option target $target ok"
    done
  done

  if [ -z "$pgno" ] ; then
    for target in 1 2 3 5 ; do
      rm exp-test-out-$target*.$module
    done
  fi
}

if [ -z "$sliced_bz2" ] ; then
  die "Please name a bzip2'ed sliced VBI file"
fi

rm -f exp-test-out-*

# Module vtx has been disabled in 0.2.28.
for module in html text ; do
  target_loop
done
for module in png ppm xpm ; do
  # xpm not ported to 0.3 yet.
  if ./export -m | grep -q $module ; then
    target_loop ,aspect=0
    target_loop ,aspect=1
  fi
done

echo "Test complete"
