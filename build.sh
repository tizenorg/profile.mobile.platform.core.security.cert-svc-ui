#!/bin/bash

if [ $1 == "app" ]
then
	echo "Build App"
	cp ./debian/control.app ./debian/control
	cp ./debian/changelog.app ./debian/changelog
	find ./debian -name "rules" -exec perl -pi -e 's/TYPE\ \?\=\ ug/TYPE\ \?\=\ app/g' {} \;
else
	echo "Build UG"
	cp ./debian/control.ug ./debian/control
	cp ./debian/changelog.ug ./debian/changelog
	find ./debian -name "rules" -exec perl -pi -e 's/TYPE\ \?\=\ app/TYPE\ \?\=\ ug/g' {} \;
fi

sbs -b
