#!/bin/bash
read option;
echo $option;
if option=1; then
 echo "git upload";
 git add .;
 git commit -m "test";
 git push;
else
 echo "git 0";
fi
