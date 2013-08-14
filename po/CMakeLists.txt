SET(POFILES
    ar.po  ca.po  de_DE.po  en.po     es_MX.po  fi.po     ga.po  hi.po  hy.po  it_IT.po  kk.po     lv.po  nb.po     pl.po     ro.po     sl.po  th.po     uz.po     zh_HK.po
    az.po  cs.po  el_GR.po  en_US.po  et.po     fr_CA.po  gl.po  hr.po  id.po  ja_JP.po  ko_KR.po  mk.po  nl_NL.po  pt_BR.po  ru_RU.po  sr.po  tr_TR.po  vi.po     zh_SG.po
    bg.po  da.po  en_PH.po  es_ES.po  eu.po     fr_FR.po  he.po  hu.po  is.po  ka.po     lt.po     ms.po  no.po     pt_PT.po  sk.po     sv.po  uk.po     zh_CN.po  zh_TW.po
)

SET(MSGFMT "/usr/bin/msgfmt")

FOREACH(pofile ${POFILES})
    SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
    MESSAGE("PO: ${pofile}")
    GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
    GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
    SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${moFile}
        COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
        DEPENDS ${absPofile}
    )
    INSTALL(FILES ${moFile}
        DESTINATION ${LOCALEDIR}/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
    SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
