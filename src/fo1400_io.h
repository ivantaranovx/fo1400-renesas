
#ifndef FO1400_IO_H
#define FO1400_IO_H

#define ON  1
#define OFF 0

/* inputs */

#define KH0     !dio_in(52)
#define KH1     !dio_in(35)
#define KH2     !dio_in(36)
#define KH3     !dio_in(37)
#define KH4     !dio_in(38)
#define KH5     !dio_in(39)
#define KH6     !dio_in(48)
#define KH7     !dio_in(49)
#define KH10    !dio_in(62)
#define KH11    !dio_in(61)
#define KH12    !dio_in(60)
#define KH13    !dio_in(59)
#define KH14    !dio_in(58)

#define OT      !dio_in(57)
#define RD      !dio_in(56)
#define CE      dio_in(5)
#define KM1     !dio_in(53)

#define M_ADJUST    !dio_in(32)
#define M_MANUAL    !dio_in(33)
#define M_SEMI_AUTO !dio_in(34)
#define M_AUTO      !dio_in(63)

#define BK1     dio_in(0)
#define BK2     !dio_in(1)
#define BK3     dio_in(2)
#define BK4     dio_in(3)

#define BK20    dio_in(18)
#define BK21    dio_in(19)
#define BK22    dio_in(20)
#define BK23    dio_in(22)
#define BK24    dio_in(21)
#define BK25    dio_in(23)

#define BK50    dio_in(16)
#define BK51    dio_in(17)
#define BK52    dio_in(7)
#define BK53    dio_in(6)

#define BK56    dio_in(54)
#define BK57    dio_in(55)
#define BK58    !dio_in(51)
#define BK59    dio_in(50)
#define BK60    dio_in(25)
#define BK61    dio_in(24)

/* outputs */

#define EM18(v)     dio_out(23, v)
#define EM29(v)     dio_out(22, v)
#define EM16(v)     dio_out(21, v)
#define EM1(v)      dio_out(20, v)
#define EM31(v)     dio_out(19, v)
#define EM13(v)     dio_out(18, v)
#define EM30(v)     dio_out(17, v)
#define EM4(v)      dio_out(16, v)

#define EM3(v)      dio_out(0, v)
#define EM6(v)      dio_out(1, v)
#define EM5(v)      dio_out(2, v)
#define EM12(v)     dio_out(3, v)
#define EM7(v)      dio_out(4, v)
#define EM2(v)      dio_out(5, v)
#define EM40(v)     dio_out(6, v)
#define EM41(v)     dio_out(7, v)

#define RDY(v)      dio_out(15, v)
#define ENGINE(v)   dio_out(14, v)
#define FAIL(v)     dio_out(13, v)

#define KM5(v)      dio_out(47, v)
#define KM10(v)     dio_out(46, v)
#define KM7(v)      dio_out(45, v)

#endif /* FO1400_IO_H */

