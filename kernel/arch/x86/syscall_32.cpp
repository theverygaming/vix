#include <vix/abi/linux/calls.h>
#include <vix/abi/linux/errno.h>
#include <vix/abi/linux/linux.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/multitasking.h>
#include <vix/arch/syscall_32.h>
#include <vix/config.h>
#include <vix/debug.h>

abi::linux::syscall_arg_t (*syscall_table[440])(abi::linux::syscall_arg_t sysarg0,
                                                abi::linux::syscall_arg_t sysarg1,
                                                abi::linux::syscall_arg_t sysarg2,
                                                abi::linux::syscall_arg_t sysarg3,
                                                abi::linux::syscall_arg_t sysarg4,
                                                abi::linux::syscall_arg_t sysarg5) = {
    nullptr,                  // 0
    &abi::linux::sys_exit,    // 1
    nullptr,                  // 2
    &abi::linux::sys_read,    // 3
    &abi::linux::sys_write,   // 4
    nullptr,                  // 5
    nullptr,                  // 6
    nullptr,                  // 7
    nullptr,                  // 8
    nullptr,                  // 9
    nullptr,                  // 10
    nullptr,                  // 11
    nullptr,                  // 12
    nullptr,                  // 13
    nullptr,                  // 14
    nullptr,                  // 15
    nullptr,                  // 16
    nullptr,                  // 17
    nullptr,                  // 18
    nullptr,                  // 19
    nullptr,                  // 20
    nullptr,                  // 21
    nullptr,                  // 22
    nullptr,                  // 23
    nullptr,                  // 24
    nullptr,                  // 25
    nullptr,                  // 26
    nullptr,                  // 27
    nullptr,                  // 28
    nullptr,                  // 29
    nullptr,                  // 30
    nullptr,                  // 31
    nullptr,                  // 32
    nullptr,                  // 33
    nullptr,                  // 34
    nullptr,                  // 35
    nullptr,                  // 36
    nullptr,                  // 37
    nullptr,                  // 38
    nullptr,                  // 39
    nullptr,                  // 40
    nullptr,                  // 41
    nullptr,                  // 42
    nullptr,                  // 43
    nullptr,                  // 44
    nullptr,                  // 45
    nullptr,                  // 46
    nullptr,                  // 47
    nullptr,                  // 48
    nullptr,                  // 49
    nullptr,                  // 50
    nullptr,                  // 51
    nullptr,                  // 52
    nullptr,                  // 53
    nullptr,                  // 54
    nullptr,                  // 55
    nullptr,                  // 56
    nullptr,                  // 57
    nullptr,                  // 58
    nullptr,                  // 59
    nullptr,                  // 60
    nullptr,                  // 61
    nullptr,                  // 62
    nullptr,                  // 63
    nullptr,                  // 64
    nullptr,                  // 65
    nullptr,                  // 66
    nullptr,                  // 67
    nullptr,                  // 68
    nullptr,                  // 69
    nullptr,                  // 70
    nullptr,                  // 71
    nullptr,                  // 72
    nullptr,                  // 73
    nullptr,                  // 74
    nullptr,                  // 75
    nullptr,                  // 76
    nullptr,                  // 77
    nullptr,                  // 78
    nullptr,                  // 79
    nullptr,                  // 80
    nullptr,                  // 81
    nullptr,                  // 82
    nullptr,                  // 83
    nullptr,                  // 84
    nullptr,                  // 85
    nullptr,                  // 86
    nullptr,                  // 87
    nullptr,                  // 88
    nullptr,                  // 89
    nullptr,                  // 90
    nullptr,                  // 91
    nullptr,                  // 92
    nullptr,                  // 93
    nullptr,                  // 94
    nullptr,                  // 95
    nullptr,                  // 96
    nullptr,                  // 97
    nullptr,                  // 98
    nullptr,                  // 99
    nullptr,                  // 100
    nullptr,                  // 101
    nullptr,                  // 102
    nullptr,                  // 103
    nullptr,                  // 104
    nullptr,                  // 105
    nullptr,                  // 106
    nullptr,                  // 107
    nullptr,                  // 108
    nullptr,                  // 109
    nullptr,                  // 110
    nullptr,                  // 111
    nullptr,                  // 112
    nullptr,                  // 113
    nullptr,                  // 114
    nullptr,                  // 115
    &abi::linux::sys_sysinfo, // 116
    nullptr,                  // 117
    nullptr,                  // 118
    nullptr,                  // 119
    nullptr,                  // 120
    nullptr,                  // 121
    nullptr,                  // 122
    nullptr,                  // 123
    nullptr,                  // 124
    nullptr,                  // 125
    nullptr,                  // 126
    nullptr,                  // 127
    nullptr,                  // 128
    nullptr,                  // 129
    nullptr,                  // 130
    nullptr,                  // 131
    nullptr,                  // 132
    nullptr,                  // 133
    nullptr,                  // 134
    nullptr,                  // 135
    nullptr,                  // 136
    nullptr,                  // 137
    nullptr,                  // 138
    nullptr,                  // 139
    nullptr,                  // 140
    nullptr,                  // 141
    nullptr,                  // 142
    nullptr,                  // 143
    nullptr,                  // 144
    nullptr,                  // 145
    nullptr,                  // 146
    nullptr,                  // 147
    nullptr,                  // 148
    nullptr,                  // 149
    nullptr,                  // 150
    nullptr,                  // 151
    nullptr,                  // 152
    nullptr,                  // 153
    nullptr,                  // 154
    nullptr,                  // 155
    nullptr,                  // 156
    nullptr,                  // 157
    nullptr,                  // 158
    nullptr,                  // 159
    nullptr,                  // 160
    nullptr,                  // 161
    nullptr,                  // 162
    nullptr,                  // 163
    nullptr,                  // 164
    nullptr,                  // 165
    nullptr,                  // 166
    nullptr,                  // 167
    nullptr,                  // 168
    nullptr,                  // 169
    nullptr,                  // 170
    nullptr,                  // 171
    nullptr,                  // 172
    nullptr,                  // 173
    nullptr,                  // 174
    nullptr,                  // 175
    nullptr,                  // 176
    nullptr,                  // 177
    nullptr,                  // 178
    nullptr,                  // 179
    nullptr,                  // 180
    nullptr,                  // 181
    nullptr,                  // 182
    nullptr,                  // 183
    nullptr,                  // 184
    nullptr,                  // 185
    nullptr,                  // 186
    nullptr,                  // 187
    nullptr,                  // 188
    nullptr,                  // 189
    nullptr,                  // 190
    nullptr,                  // 191
    nullptr,                  // 192
    nullptr,                  // 193
    nullptr,                  // 194
    nullptr,                  // 195
    nullptr,                  // 196
    nullptr,                  // 197
    nullptr,                  // 198
    nullptr,                  // 199
    nullptr,                  // 200
    nullptr,                  // 201
    nullptr,                  // 202
    nullptr,                  // 203
    nullptr,                  // 204
    nullptr,                  // 205
    nullptr,                  // 206
    nullptr,                  // 207
    nullptr,                  // 208
    nullptr,                  // 209
    nullptr,                  // 210
    nullptr,                  // 211
    nullptr,                  // 212
    nullptr,                  // 213
    nullptr,                  // 214
    nullptr,                  // 215
    nullptr,                  // 216
    nullptr,                  // 217
    nullptr,                  // 218
    nullptr,                  // 219
    nullptr,                  // 220
    nullptr,                  // 221
    nullptr,                  // 222
    nullptr,                  // 223
    nullptr,                  // 224
    nullptr,                  // 225
    nullptr,                  // 226
    nullptr,                  // 227
    nullptr,                  // 228
    nullptr,                  // 229
    nullptr,                  // 230
    nullptr,                  // 231
    nullptr,                  // 232
    nullptr,                  // 233
    nullptr,                  // 234
    nullptr,                  // 235
    nullptr,                  // 236
    nullptr,                  // 237
    nullptr,                  // 238
    nullptr,                  // 239
    nullptr,                  // 240
    nullptr,                  // 241
    nullptr,                  // 242
    nullptr,                  // 243
    nullptr,                  // 244
    nullptr,                  // 245
    nullptr,                  // 246
    nullptr,                  // 247
    nullptr,                  // 248
    nullptr,                  // 249
    nullptr,                  // 250
    nullptr,                  // 251
    nullptr,                  // 252
    nullptr,                  // 253
    nullptr,                  // 254
    nullptr,                  // 255
    nullptr,                  // 256
    nullptr,                  // 257
    nullptr,                  // 258
    nullptr,                  // 259
    nullptr,                  // 260
    nullptr,                  // 261
    nullptr,                  // 262
    nullptr,                  // 263
    nullptr,                  // 264
    nullptr,                  // 265
    nullptr,                  // 266
    nullptr,                  // 267
    nullptr,                  // 268
    nullptr,                  // 269
    nullptr,                  // 270
    nullptr,                  // 271
    nullptr,                  // 272
    nullptr,                  // 273
    nullptr,                  // 274
    nullptr,                  // 275
    nullptr,                  // 276
    nullptr,                  // 277
    nullptr,                  // 278
    nullptr,                  // 279
    nullptr,                  // 280
    nullptr,                  // 281
    nullptr,                  // 282
    nullptr,                  // 283
    nullptr,                  // 284
    nullptr,                  // 285
    nullptr,                  // 286
    nullptr,                  // 287
    nullptr,                  // 288
    nullptr,                  // 289
    nullptr,                  // 290
    nullptr,                  // 291
    nullptr,                  // 292
    nullptr,                  // 293
    nullptr,                  // 294
    nullptr,                  // 295
    nullptr,                  // 296
    nullptr,                  // 297
    nullptr,                  // 298
    nullptr,                  // 299
    nullptr,                  // 300
    nullptr,                  // 301
    nullptr,                  // 302
    nullptr,                  // 303
    nullptr,                  // 304
    nullptr,                  // 305
    nullptr,                  // 306
    nullptr,                  // 307
    nullptr,                  // 308
    nullptr,                  // 309
    nullptr,                  // 310
    nullptr,                  // 311
    nullptr,                  // 312
    nullptr,                  // 313
    nullptr,                  // 314
    nullptr,                  // 315
    nullptr,                  // 316
    nullptr,                  // 317
    nullptr,                  // 318
    nullptr,                  // 319
    nullptr,                  // 320
    nullptr,                  // 321
    nullptr,                  // 322
    nullptr,                  // 323
    nullptr,                  // 324
    nullptr,                  // 325
    nullptr,                  // 326
    nullptr,                  // 327
    nullptr,                  // 328
    nullptr,                  // 329
    nullptr,                  // 330
    nullptr,                  // 331
    nullptr,                  // 332
    nullptr,                  // 333
    nullptr,                  // 334
    nullptr,                  // 335
    nullptr,                  // 336
    nullptr,                  // 337
    nullptr,                  // 338
    nullptr,                  // 339
    nullptr,                  // 340
    nullptr,                  // 341
    nullptr,                  // 342
    nullptr,                  // 343
    nullptr,                  // 344
    nullptr,                  // 345
    nullptr,                  // 346
    nullptr,                  // 347
    nullptr,                  // 348
    nullptr,                  // 349
    nullptr,                  // 350
    nullptr,                  // 351
    nullptr,                  // 352
    nullptr,                  // 353
    nullptr,                  // 354
    nullptr,                  // 355
    nullptr,                  // 356
    nullptr,                  // 357
    nullptr,                  // 358
    nullptr,                  // 359
    nullptr,                  // 360
    nullptr,                  // 361
    nullptr,                  // 362
    nullptr,                  // 363
    nullptr,                  // 364
    nullptr,                  // 365
    nullptr,                  // 366
    nullptr,                  // 367
    nullptr,                  // 368
    nullptr,                  // 369
    nullptr,                  // 370
    nullptr,                  // 371
    nullptr,                  // 372
    nullptr,                  // 373
    nullptr,                  // 374
    nullptr,                  // 375
    nullptr,                  // 376
    nullptr,                  // 377
    nullptr,                  // 378
    nullptr,                  // 379
    nullptr,                  // 380
    nullptr,                  // 381
    nullptr,                  // 382
    nullptr,                  // 383
    nullptr,                  // 384
    nullptr,                  // 385
    nullptr,                  // 386
    nullptr,                  // 387
    nullptr,                  // 388
    nullptr,                  // 389
    nullptr,                  // 390
    nullptr,                  // 391
    nullptr,                  // 392
    nullptr,                  // 393
    nullptr,                  // 394
    nullptr,                  // 395
    nullptr,                  // 396
    nullptr,                  // 397
    nullptr,                  // 398
    nullptr,                  // 399
    nullptr,                  // 400
    nullptr,                  // 401
    nullptr,                  // 402
    nullptr,                  // 403
    nullptr,                  // 404
    nullptr,                  // 405
    nullptr,                  // 406
    nullptr,                  // 407
    nullptr,                  // 408
    nullptr,                  // 409
    nullptr,                  // 410
    nullptr,                  // 411
    nullptr,                  // 412
    nullptr,                  // 413
    nullptr,                  // 414
    nullptr,                  // 415
    nullptr,                  // 416
    nullptr,                  // 417
    nullptr,                  // 418
    nullptr,                  // 419
    nullptr,                  // 420
    nullptr,                  // 421
    nullptr,                  // 422
    nullptr,                  // 423
    nullptr,                  // 424
    nullptr,                  // 425
    nullptr,                  // 426
    nullptr,                  // 427
    nullptr,                  // 428
    nullptr,                  // 429
    nullptr,                  // 430
    nullptr,                  // 431
    nullptr,                  // 432
    nullptr,                  // 433
    nullptr,                  // 434
    nullptr,                  // 435
    nullptr,                  // 436
    nullptr,                  // 437
    nullptr,                  // 438
    nullptr,                  // 439
};

extern "C" void syscallHandler(struct arch::full_ctx *regs) {
    if (regs->eax <= (sizeof(syscall_table) / sizeof(syscall_table[0]))) {
        kprintf(KP_INFO, "calling syscall %u\n", regs->eax);
        abi::linux::syscall_arg_t retval;
        if (syscall_table[regs->eax] == nullptr) {
            kprintf(KP_INFO, "syscall %u not found\n", regs->eax);
            regs->eax = -ENOSYS;
            return;
        }
        retval = syscall_table[regs->eax]((abi::linux::syscall_arg_t)regs->ebx,
                                          (abi::linux::syscall_arg_t)regs->ecx,
                                          (abi::linux::syscall_arg_t)regs->edx,
                                          (abi::linux::syscall_arg_t)regs->esi,
                                          (abi::linux::syscall_arg_t)regs->edi,
                                          (abi::linux::syscall_arg_t)regs->ebp);
        kprintf(KP_INFO, "syscall return %d\n", retval);
        regs->eax = retval;
        return;
    }
    kprintf(KP_INFO, "syscall %u not found\n", regs->eax);
    regs->eax = -ENOSYS;
}
