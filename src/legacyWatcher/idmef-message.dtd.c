/* This is most of the idmef DTD, with the comments removed
 * it is used to initialize the compression algorithm before 
 * giving it a message, in the hopes that it will improve the
 * compression ratio.  (since by then the compression will
 * have already seen all the various normal XML words.)
 *
 * $Id: idmef-message.dtd.c,v 1.2 2005/11/28 17:53:48 tjohnson Exp $
 */

#define dictionary "\
<!ENTITY % attlist.idmef                \
    version             CDATA                   #FIXED    '1.0'\
  >\
\
\
<!ENTITY % attlist.global               \
    xmlns:idmef         CDATA                   #FIXED\
        'urn:iana:xml:ns:idmef'\
    xmlns               CDATA                   #FIXED\
        'urn:iana:xml:ns:idmef'\
    xml:space           (default | preserve)    'default'\
    xml:lang            NMTOKEN                 #IMPLIED\
  >\
\
\
\
<!ENTITY % attvals.actioncat            \
    ( block-installed | notification-sent | taken-offline | other )\
  >\
\
\
<!ENTITY % attvals.addrcat              \
    ( unknown | atm | e-mail | lotus-notes | mac | sna | vm |\
      ipv4-addr | ipv4-addr-hex | ipv4-net | ipv4-net-mask |\
      ipv6-addr | ipv6-addr-hex | ipv6-net | ipv6-net-mask )\
  >\
\
\
<!ENTITY % attvals.adtype               \
    ( boolean | byte | character | date-time | integer | ntpstamp |\
      portlist | real | string | xml )\
  >\
\
\
<!ENTITY % attvals.completion           \
    ( failed | succeeded )\
  >\
\
\
<!ENTITY % attvals.filecat              \
    ( current | original )\
  >\
\
\
<!ENTITY % attvals.idtype               \
    ( current-user | original-user | target-user | user-privs |\
      current-group | group-privs | other-privs )\
  >\
\
\
<!ENTITY % attvals.impacttype           \
    ( admin | dos | file | recon | user | other )\
  >\
\
\
<!ENTITY % attvals.linkcat              \
    ( hard-link | mount-point | reparse-point | shortcut | stream |\
      symbolic-link )\
  >\
\
\
<!ENTITY % attvals.checksumalgos        \
      ( MD4 | MD5 | SHA1 | SHA2-256 | SHA2-384 | SHA2-512 | CRC-32 | Haval | Tiger | Gost )\
  >\
\
\
<!ENTITY % attvals.nodecat              \
    ( unknown | ads | afs | coda | dfs | dns | hosts | kerberos |\
      nds | nis | nisplus | nt | wfw )\
  >\
\
\
<!ENTITY % attvals.origin               \
    ( unknown | vendor-specific | user-specific | bugtraqid | cve | osvdb )\
  >\
\
\
<!ENTITY % attvals.rating               \
    ( low | medium | high | numeric )\
  >\
\
\
<!ENTITY % attvals.severity             \
    ( info | low | medium | high )\
  >\
\
\
<!ENTITY % attvals.usercat              \
    ( unknown | application | os-device )\
  >\
\
\
<!ENTITY % attvals.yesno                \
    ( unknown | yes | no )\
  >\
\
\
\
<!ELEMENT IDMEF-Message                 (\
    (Alert | Heartbeat)*\
  )>\
<!ATTLIST IDMEF-Message\
    %attlist.global;\
    %attlist.idmef;\
  >\
\
\
<!ELEMENT Alert                         (\
    Analyzer, CreateTime, DetectTime?, AnalyzerTime?,\
    Source*, Target*, Classification, Assessment?, (ToolAlert |\
    OverflowAlert | CorrelationAlert)?, AdditionalData*\
  )>\
<!ATTLIST Alert\
    messageid           CDATA                   '0'\
    %attlist.global;\
  >\
\
\
<!ELEMENT Heartbeat                     (\
    Analyzer, CreateTime, AnalyzerTime?, AdditionalData*\
  )>\
<!ATTLIST Heartbeat\
    messageid           CDATA                   '0'\
    %attlist.global;\
  >\
\
\
\
<!ELEMENT CorrelationAlert              (\
    name, alertident+\
  )>\
<!ATTLIST CorrelationAlert\
    %attlist.global;\
  >\
\
\
<!ELEMENT OverflowAlert                 (\
    program, size?, buffer?\
  )>\
<!ATTLIST OverflowAlert\
    %attlist.global;\
  >\
\
\
<!ELEMENT ToolAlert                     (\
    name, command?, alertident+\
  )>\
<!ATTLIST ToolAlert\
    %attlist.global;\
  >\
\
\
<!ELEMENT AdditionalData            ANY >\
<!ATTLIST AdditionalData\
    type                %attvals.adtype;        'string'\
    meaning             CDATA                   #IMPLIED\
    %attlist.global;\
  >\
\
\
<!ELEMENT Analyzer                      (\
    Node?, Process?, Analyzer?\
  )>\
<!ATTLIST Analyzer\
    analyzerid          CDATA                   '0'\
    name                CDATA                   #IMPLIED\
    manufacturer        CDATA                   #IMPLIED\
    model               CDATA                   #IMPLIED\
    version             CDATA                   #IMPLIED\
    class               CDATA                   #IMPLIED\
    ostype              CDATA                   #IMPLIED\
    osversion           CDATA                   #IMPLIED\
    %attlist.global;\
  >\
\
\
<!ELEMENT Source                        (\
    Node?, User?, Process?, Service?\
  )>\
<!ATTLIST Source\
    ident               CDATA                   '0'\
    spoofed             %attvals.yesno;         'unknown'\
    interface           CDATA                   #IMPLIED\
    %attlist.global;\
  >\
\
\
<!ELEMENT Target                        (\
    Node?, User?, Process?, Service?, FileList?\
  )>\
<!ATTLIST Target\
    ident               CDATA                   '0'\
    decoy               %attvals.yesno;         'unknown'\
    interface           CDATA                   #IMPLIED\
    %attlist.global;\
  >\
\
\
<!ELEMENT Address                       (\
    address, netmask?\
  )>\
<!ATTLIST Address\
    ident               CDATA                   '0'\
    category            %attvals.addrcat;       'unknown'\
    vlan-name           CDATA                   #IMPLIED\
    vlan-num            CDATA                   #IMPLIED\
    %attlist.global;\
  >\
\
\
<!ELEMENT Assessment                    (\
    Impact?, Action*, Confidence?\
  )>\
<!ATTLIST Assessment\
    %attlist.global;\
  >\
\
\
<!ELEMENT Classification                (\
    Reference*\
  )>\
<!ATTLIST Classification\
    ident               CDATA                   '0'\
    text                CDATA                   #REQUIRED\
  >\
\
\
<!ELEMENT Reference                (\
    name, url\
  )>\
<!ATTLIST Reference\
    origin              %attvals.origin;        'unknown'\
    meaning             CDATA                   #IMPLIED\
  >\
\
\
<!ELEMENT FileList                      (\
    File+\
  )>\
<!ATTLIST FileList\
    %attlist.global;\
  >\
\
\
<!ELEMENT File                          (\
    name, path, create-time?, modify-time?, access-time?,\
    data-size?, disk-size?, FileAccess*, Linkage*, Inode?,\
    Checksum*\
  )>\
<!ATTLIST File\
    ident               CDATA                   '0'\
    category            %attvals.filecat;       #REQUIRED\
    fstype              CDATA                   #IMPLIED\
    %attlist.global;\
  >\
\
\
<!ELEMENT FileAccess                    (\
    UserId, permission+\
  )>\
<!ATTLIST FileAccess\
    %attlist.global;\
  >\
\
\
<!ELEMENT Inode                         (\
    change-time?, (number, major-device, minor-device)?,\
    (c-major-device, c-minor-device)?\
  )>\
<!ATTLIST Inode\
    %attlist.global;\
  >\
\
\
<!ELEMENT Linkage                       (\
    (name, path) | File\
  )>\
<!ATTLIST Linkage\
    category            %attvals.linkcat;       #REQUIRED\
    %attlist.global;\
  >\
\
\
<!ELEMENT Checksum                      (\
    value, key?\
  )>\
<!ATTLIST Checksum\
    algorithm           %attvals.checksumalgos; #REQUIRED\
    %attlist.global;\
  >\
\
\
<!ELEMENT Node                          (\
    location?, (name | Address), Address*\
  )>\
<!ATTLIST Node\
    ident               CDATA                   '0'\
    category            %attvals.nodecat;       'unknown'\
    %attlist.global;\
  >\
\
\
<!ELEMENT Process                       (\
    name, pid?, path?, arg*, env*\
  )>\
<!ATTLIST Process\
    ident               CDATA                   '0'\
    %attlist.global;\
  >\
\
\
<!ELEMENT Service                       (\
    (((name, port?) | (port, name?)) | portlist), protocol?,\
    SNMPService?, WebService?\
  )>\
<!ATTLIST Service\
    ident                CDATA                   '0'\
     ip_version           CDATA                   #IMPLIED\
     iana_protocol_number CDATA                  #IMPLIED\
     iana_protocol_name   CDATA                  #IMPLIED\
    %attlist.global;\
  >\
\
\
<!ELEMENT SNMPService                   (\
    oid?, (community | (securityName, contextName,\
    contextEngineID))?, command?\
  )>\
<!ATTLIST SNMPService\
    %attlist.global;\
  >\
\
\
<!ELEMENT User                          (\
    UserId+\
  )>\
<!ATTLIST User\
    ident               CDATA                   '0'\
    category            %attvals.usercat;       'unknown'\
    %attlist.global;\
  >\
\
\
<!ELEMENT UserId                        (\
    (name, number?) | (number, name?)\
  )>\
<!ATTLIST UserId\
    ident               CDATA                   '0'\
    type                %attvals.idtype;        'original-user'\
    %attlist.global;\
  >\
\
\
<!ELEMENT WebService                    (\
    url, cgi?, http-method?, arg*\
  )>\
<!ATTLIST WebService\
    %attlist.global;\
  >\
\
<!ELEMENT Action              (#PCDATA) >\
<!ATTLIST Action\
    category            %attvals.actioncat;     'other'\
    %attlist.global;\
  >\
\
\
<!ELEMENT AnalyzerTime        (#PCDATA) >\
<!ATTLIST AnalyzerTime\
    ntpstamp            CDATA                   #REQUIRED\
    %attlist.global;\
  >\
\
\
<!ELEMENT Confidence          (#PCDATA) >\
<!ATTLIST Confidence\
    rating              %attvals.rating;        'numeric'\
    %attlist.global;\
  >\
\
\
<!ELEMENT CreateTime          (#PCDATA) >\
<!ATTLIST CreateTime\
    ntpstamp            CDATA                   #REQUIRED\
    %attlist.global;\
  >\
\
\
<!ELEMENT DetectTime          (#PCDATA) >\
<!ATTLIST DetectTime\
    ntpstamp            CDATA                   #REQUIRED\
    %attlist.global;\
\
\
  >\
\
\
<!ELEMENT Impact              (#PCDATA) >\
<!ATTLIST Impact\
    severity            %attvals.severity;      #IMPLIED\
    completion          %attvals.completion;    #IMPLIED\
    type                %attvals.impacttype;    'other'\
    %attlist.global;\
  >\
\
\
<!ELEMENT alertident          (#PCDATA) >\
<!ATTLIST alertident\
    analyzerid          CDATA                   #IMPLIED\
    %attlist.global;\
  >\
\
\
<!ELEMENT access-time         (#PCDATA) >\
<!ATTLIST access-time\
    %attlist.global;\
  >\
\
\
<!ELEMENT address             (#PCDATA) >\
<!ATTLIST address\
    %attlist.global;\
  >\
\
\
<!ELEMENT arg                 (#PCDATA) >\
<!ATTLIST arg\
    %attlist.global;\
  >\
\
\
<!ELEMENT buffer              (#PCDATA) >\
<!ATTLIST buffer\
    %attlist.global;\
  >\
\
\
<!ELEMENT c-major-device      (#PCDATA) >\
<!ATTLIST c-major-device\
    %attlist.global;\
  >\
\
\
<!ELEMENT c-minor-device      (#PCDATA) >\
<!ATTLIST c-minor-device\
    %attlist.global;\
  >\
\
\
<!ELEMENT cgi                 (#PCDATA) >\
<!ATTLIST cgi\
    %attlist.global;\
  >\
\
\
<!ELEMENT change-time         (#PCDATA) >\
<!ATTLIST change-time\
    %attlist.global;\
  >\
\
\
<!ELEMENT command             (#PCDATA) >\
<!ATTLIST command\
    %attlist.global;\
  >\
\
\
<!ELEMENT community           (#PCDATA) >\
<!ATTLIST community\
    %attlist.global;\
  >\
\
\
<!ELEMENT contextEngineID     (#PCDATA) >\
<!ATTLIST contextEngineID\
    %attlist.global;\
  >\
\
\
<!ELEMENT contextName         (#PCDATA) >\
<!ATTLIST contextName\
    %attlist.global;\
  >\
\
\
<!ELEMENT create-time         (#PCDATA) >\
<!ATTLIST create-time\
    %attlist.global;\
  >\
\
\
<!ELEMENT data-size           (#PCDATA) >\
<!ATTLIST data-size\
    %attlist.global;\
  >\
\
\
<!ELEMENT disk-size           (#PCDATA) >\
<!ATTLIST disk-size\
    %attlist.global;\
  >\
\
\
<!ELEMENT env                 (#PCDATA) >\
<!ATTLIST env\
    %attlist.global;\
  >\
\
\
<!ELEMENT http-method         (#PCDATA) >\
<!ATTLIST http-method\
    %attlist.global;\
  >\
\
\
<!ELEMENT key                 (#PCDATA) >\
<!ATTLIST key\
    %attlist.global;\
  >\
\
<!ELEMENT location            (#PCDATA) >\
<!ATTLIST location\
    %attlist.global;\
  >\
\
\
<!ELEMENT major-device        (#PCDATA) >\
<!ATTLIST major-device\
    %attlist.global;\
  >\
\
\
<!ELEMENT minor-device        (#PCDATA) >\
<!ATTLIST minor-device\
    %attlist.global;\
  >\
\
\
<!ELEMENT modify-time         (#PCDATA) >\
<!ATTLIST modify-time\
    %attlist.global;\
  >\
\
\
<!ELEMENT name                (#PCDATA) >\
<!ATTLIST name\
    %attlist.global;\
  >\
\
\
<!ELEMENT netmask             (#PCDATA) >\
<!ATTLIST netmask\
    %attlist.global;\
  >\
\
\
<!ELEMENT number              (#PCDATA) >\
<!ATTLIST number\
    %attlist.global;\
  >\
\
\
<!ELEMENT oid                 (#PCDATA) >\
<!ATTLIST oid\
    %attlist.global;\
  >\
\
\
<!ELEMENT path                (#PCDATA) >\
<!ATTLIST path\
    %attlist.global;\
  >\
\
\
<!ELEMENT permission          (#PCDATA) >\
<!ATTLIST permission\
    %attlist.global;\
  >\
\
\
<!ELEMENT pid                 (#PCDATA) >\
<!ATTLIST pid\
    %attlist.global;\
  >\
\
<!ELEMENT port                (#PCDATA) >\
<!ATTLIST port\
    %attlist.global;\
  >\
\
\
<!ELEMENT portlist            (#PCDATA) >\
<!ATTLIST portlist\
    %attlist.global;\
  >\
\
\
<!ELEMENT program             (#PCDATA) >\
<!ATTLIST program\
    %attlist.global;\
  >\
\
\
<!ELEMENT protocol            (#PCDATA) >\
<!ATTLIST protocol\
    %attlist.global;\
  >\
\
<!ELEMENT securityName        (#PCDATA) >\
<!ATTLIST securityName\
    %attlist.global;\
  >\
\
<!ELEMENT size                (#PCDATA) >\
<!ATTLIST size\
    %attlist.global;\
  >\
\
\
<!ELEMENT url                 (#PCDATA) >\
<!ATTLIST url\
    %attlist.global;\
  >\
\
<!ELEMENT value               (#PCDATA) >\
<!ATTLIST value\
    %attlist.global;\
  >\
"
