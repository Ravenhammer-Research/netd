#define IDENTIFIER 257
#define IPV4_ADDRESS 258
#define IPV6_ADDRESS 259
#define CIDR_ADDRESS 260
#define NUMBER 261
#define STRING 262
#define PROTOCOL 263
#define SET 264
#define SHOW 265
#define DELETE 266
#define COMMIT 267
#define SAVE 268
#define VRF 269
#define INTERFACE 270
#define ROUTE 271
#define ETHERNET 272
#define WIRELESS80211 273
#define EPAIR 274
#define GIF 275
#define GRE 276
#define LAGG 277
#define LO 278
#define OVPN 279
#define TUN 280
#define TAP 281
#define VLAN 282
#define VXLAN 283
#define INET 284
#define INET6 285
#define TABLE 286
#define STATIC 287
#define ADDRESS 288
#define MTU 289
#define GROUP 290
#define REJECT_TOKEN 291
#define BLACKHOLE 292
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
    char *string;
    int integer;
    command_type_t cmd_type;
    object_type_t obj_type;
    interface_type_t if_type;
    int family;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE yylval;
