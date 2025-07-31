#define IDENTIFIER 257
#define IPV4_ADDRESS 258
#define IPV6_ADDRESS 259
#define CIDR_ADDRESS 260
#define NUMBER 261
#define STRING 262
#define SET 263
#define SHOW 264
#define DELETE 265
#define COMMIT 266
#define SAVE 267
#define VRF 268
#define INTERFACE 269
#define ROUTE 270
#define ETHERNET 271
#define WIRELESS80211 272
#define EPAIR 273
#define GIF 274
#define GRE 275
#define LAGG 276
#define LO 277
#define OVPN 278
#define TUN 279
#define TAP 280
#define VLAN 281
#define VXLAN 282
#define INET 283
#define INET6 284
#define TABLE 285
#define STATIC 286
#define ADDRESS 287
#define MTU 288
#define GROUP 289
#define REJECT_TOKEN 290
#define BLACKHOLE 291
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
