#!/usr/bin/env bash

DOCDIR="/tmp/cphvbdocs"

if [ -d "$DOCDIR" ]; then       # Grab the repos
    echo "Already got repos, not cloning."
else
    git clone git@bitbucket.org:cphvb/cphvb.bitbucket.org.git $DOCDIR
fi

doxygen Doxyfile                # Generate C++ docs
make html                       # Generate HTML via sphinx

cp -r build/html/* $DOCDIR      # Copy to repos
cd $DOCDIR
git add .
git commit -a -m "Autogenerated docs."
git push                        # Send it off
