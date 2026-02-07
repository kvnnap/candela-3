#!/bin/sh
echo -n "#define CANDELA_COMMIT " > version/auto_version.h
git rev-parse --verify HEAD >> version/auto_version.h
echo -n "#define CANDELA_DATE " >> version/auto_version.h
git log -1 --format=%ci >> version/auto_version.h
git diff --quiet || echo "#define CANDELA_DIRTY" >> version/auto_version.h

