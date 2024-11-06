; ModuleID = 'test_module'
source_filename = "test_module"

@0 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%f\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@3 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

declare i32 @printf(ptr, ...)

define void @main() {
entry:
  br label %while_condition

while_condition:                                  ; preds = %unreachable, %entry
  br i1 true, label %while_stmt, label %while_done

while_stmt:                                       ; preds = %while_condition
  %strtmp = alloca [6 x i8], align 1
  store [6 x i8] c"first\00", ptr %strtmp, align 1
  %0 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp)
  %1 = call i32 (ptr, ...) @printf(ptr @3)
  br label %while_done

while_done:                                       ; preds = %while_stmt, %while_condition
  br i1 false, label %then, label %if_cont

unreachable:                                      ; No predecessors!
  %strtmp1 = alloca [7 x i8], align 1
  store [7 x i8] c"second\00", ptr %strtmp1, align 1
  %2 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp1)
  %3 = call i32 (ptr, ...) @printf(ptr @3)
  br label %while_condition

then:                                             ; preds = %while_done
  %strtmp2 = alloca [3 x i8], align 1
  store [3 x i8] c"if\00", ptr %strtmp2, align 1
  %4 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp2)
  %5 = call i32 (ptr, ...) @printf(ptr @3)
  br label %if_cont

if_cont:                                          ; preds = %then, %while_done
  %strtmp3 = alloca [5 x i8], align 1
  store [5 x i8] c"else\00", ptr %strtmp3, align 1
  %6 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp3)
  %7 = call i32 (ptr, ...) @printf(ptr @3)
  br i1 true, label %then4, label %else

then4:                                            ; preds = %if_cont
  %strtmp6 = alloca [3 x i8], align 1
  store [3 x i8] c"if\00", ptr %strtmp6, align 1
  %8 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp6)
  %9 = call i32 (ptr, ...) @printf(ptr @3)
  br label %if_cont5

else:                                             ; preds = %if_cont
  %strtmp7 = alloca [5 x i8], align 1
  store [5 x i8] c"else\00", ptr %strtmp7, align 1
  %10 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp7)
  %11 = call i32 (ptr, ...) @printf(ptr @3)
  br label %if_cont5

if_cont5:                                         ; preds = %else, %then4
  br i1 true, label %then8, label %else9

then8:                                            ; preds = %if_cont5
  %strtmp11 = alloca [3 x i8], align 1
  store [3 x i8] c"if\00", ptr %strtmp11, align 1
  %12 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp11)
  %13 = call i32 (ptr, ...) @printf(ptr @3)
  br label %if_cont10

else9:                                            ; preds = %if_cont5
  br i1 true, label %then12, label %else13

if_cont10:                                        ; preds = %if_cont14, %then8
  %14 = call i32 (ptr, ...) @printf(ptr @0, i32 12)
  %15 = call i32 (ptr, ...) @printf(ptr @3)
  %16 = call i32 (ptr, ...) @printf(ptr @1, double 1.400000e+01)
  %17 = call i32 (ptr, ...) @printf(ptr @3)
  %18 = call i32 (ptr, ...) @printf(ptr @0, i1 true)
  %19 = call i32 (ptr, ...) @printf(ptr @3)
  %20 = call i32 (ptr, ...) @printf(ptr @0, i1 false)
  %21 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp17 = alloca [6 x i8], align 1
  store [6 x i8] c"hello\00", ptr %strtmp17, align 1
  %strtmp18 = alloca [2 x i8], align 1
  store [2 x i8] c" \00", ptr %strtmp18, align 1
  %strtmp19 = alloca [6 x i8], align 1
  store [6 x i8] c"world\00", ptr %strtmp19, align 1
  %22 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp17)
  %23 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp18)
  %24 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp19)
  %25 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp20 = alloca [6 x i8], align 1
  store [6 x i8] c"world\00", ptr %strtmp20, align 1
  %26 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp20)
  %27 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp21 = alloca [25 x i8], align 1
  store [25 x i8] c"print 1.3 > 2.1 ? 1 : 2;\00", ptr %strtmp21, align 1
  %28 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp21)
  %29 = call i32 (ptr, ...) @printf(ptr @3)
  br i1 false, label %true, label %false

then12:                                           ; preds = %else9
  %strtmp15 = alloca [8 x i8], align 1
  store [8 x i8] c"else if\00", ptr %strtmp15, align 1
  %30 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp15)
  %31 = call i32 (ptr, ...) @printf(ptr @3)
  br label %if_cont14

else13:                                           ; preds = %else9
  %strtmp16 = alloca [5 x i8], align 1
  store [5 x i8] c"else\00", ptr %strtmp16, align 1
  %32 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp16)
  %33 = call i32 (ptr, ...) @printf(ptr @3)
  br label %if_cont14

if_cont14:                                        ; preds = %else13, %then12
  br label %if_cont10

true:                                             ; preds = %if_cont10
  br label %done

false:                                            ; preds = %if_cont10
  br label %done

done:                                             ; preds = %false, %true
  %"ternary result" = phi i32 [ 1, %true ], [ 2, %false ]
  %34 = call i32 (ptr, ...) @printf(ptr @0, i32 %"ternary result")
  %35 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp22 = alloca [26 x i8], align 1
  store [26 x i8] c"print 1.4 >= 3.5 ? 1 : 2;\00", ptr %strtmp22, align 1
  %36 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp22)
  %37 = call i32 (ptr, ...) @printf(ptr @3)
  br i1 false, label %true23, label %false24

true23:                                           ; preds = %done
  br label %done25

false24:                                          ; preds = %done
  br label %done25

done25:                                           ; preds = %false24, %true23
  %"ternary result26" = phi i32 [ 1, %true23 ], [ 2, %false24 ]
  %38 = call i32 (ptr, ...) @printf(ptr @0, i32 %"ternary result26")
  %39 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp27 = alloca [21 x i8], align 1
  store [21 x i8] c"print 1 < 2 ? 1 : 2;\00", ptr %strtmp27, align 1
  %40 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp27)
  %41 = call i32 (ptr, ...) @printf(ptr @3)
  br i1 true, label %true28, label %false29

true28:                                           ; preds = %done25
  br label %done30

false29:                                          ; preds = %done25
  br label %done30

done30:                                           ; preds = %false29, %true28
  %"ternary result31" = phi i32 [ 1, %true28 ], [ 2, %false29 ]
  %42 = call i32 (ptr, ...) @printf(ptr @0, i32 %"ternary result31")
  %43 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp32 = alloca [26 x i8], align 1
  store [26 x i8] c"print 6.1 <= 4.2 ? 1 : 2;\00", ptr %strtmp32, align 1
  %44 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp32)
  %45 = call i32 (ptr, ...) @printf(ptr @3)
  br i1 false, label %true33, label %false34

true33:                                           ; preds = %done30
  br label %done35

false34:                                          ; preds = %done30
  br label %done35

done35:                                           ; preds = %false34, %true33
  %"ternary result36" = phi i32 [ 1, %true33 ], [ 2, %false34 ]
  %46 = call i32 (ptr, ...) @printf(ptr @0, i32 %"ternary result36")
  %47 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp37 = alloca [26 x i8], align 1
  store [26 x i8] c"print 1.4 == 5.7 ? 1 : 2;\00", ptr %strtmp37, align 1
  %48 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp37)
  %49 = call i32 (ptr, ...) @printf(ptr @3)
  br i1 false, label %true38, label %false39

true38:                                           ; preds = %done35
  br label %done40

false39:                                          ; preds = %done35
  br label %done40

done40:                                           ; preds = %false39, %true38
  %"ternary result41" = phi i32 [ 1, %true38 ], [ 2, %false39 ]
  %50 = call i32 (ptr, ...) @printf(ptr @0, i32 %"ternary result41")
  %51 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp42 = alloca [26 x i8], align 1
  store [26 x i8] c"print 6.3 != 8.7 ? 1 : 2;\00", ptr %strtmp42, align 1
  %52 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp42)
  %53 = call i32 (ptr, ...) @printf(ptr @3)
  br i1 true, label %true43, label %false44

true43:                                           ; preds = %done40
  br label %done45

false44:                                          ; preds = %done40
  br label %done45

done45:                                           ; preds = %false44, %true43
  %"ternary result46" = phi i32 [ 1, %true43 ], [ 2, %false44 ]
  %54 = call i32 (ptr, ...) @printf(ptr @0, i32 %"ternary result46")
  %55 = call i32 (ptr, ...) @printf(ptr @3)
  call void @printDoubleF(double 2.000000e+00)
  call void @printDoubleF(double 1.200000e+00)
  %strtmp47 = alloca [20 x i8], align 1
  store [20 x i8] c"the double of 2 is \00", ptr %strtmp47, align 1
  %calltmp = call i32 @double(i32 2)
  %56 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp47)
  %57 = call i32 (ptr, ...) @printf(ptr @0, i32 %calltmp)
  %58 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp48 = alloca [23 x i8], align 1
  store [23 x i8] c"the quadruple of 2 is \00", ptr %strtmp48, align 1
  %calltmp49 = call i32 @quadruple(i32 2)
  %59 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp48)
  %60 = call i32 (ptr, ...) @printf(ptr @0, i32 %calltmp49)
  %61 = call i32 (ptr, ...) @printf(ptr @3)
  %calltmp50 = call i32 @double(i32 2)
  %x = alloca i32, align 4
  store i32 %calltmp50, ptr %x, align 4
  %x51 = load i32, ptr %x, align 4
  %calltmp52 = call i32 @double(i32 %x51)
  %62 = call i32 (ptr, ...) @printf(ptr @0, i32 %calltmp52)
  %63 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp53 = alloca [9 x i8], align 1
  store [9 x i8] c"for loop\00", ptr %strtmp53, align 1
  %64 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp53)
  %65 = call i32 (ptr, ...) @printf(ptr @3)
  %z = alloca i32, align 4
  store i32 0, ptr %z, align 4
  br label %for_initializer

for_initializer:                                  ; preds = %done45
  %z54 = load i32, ptr %z, align 4
  %y = alloca i32, align 4
  store i32 %z54, ptr %y, align 4
  br label %for_condition

for_condition:                                    ; preds = %for_increment, %for_initializer
  %y55 = load i32, ptr %y, align 4
  %sletmp = icmp sle i32 %y55, 5
  br i1 %sletmp, label %for_body, label %for_merge

for_increment:                                    ; preds = %for_body
  %loadtmp59 = load i32, ptr %y, align 4
  %addtmp = add i32 %loadtmp59, 1
  store i32 %addtmp, ptr %y, align 4
  br label %for_condition

for_body:                                         ; preds = %for_condition
  %loadtmp = load i32, ptr %z, align 4
  %y56 = load i32, ptr %y, align 4
  store i32 %y56, ptr %z, align 4
  %strtmp57 = alloca [5 x i8], align 1
  store [5 x i8] c"z = \00", ptr %strtmp57, align 1
  %z58 = load i32, ptr %z, align 4
  %66 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp57)
  %67 = call i32 (ptr, ...) @printf(ptr @0, i32 %z58)
  %68 = call i32 (ptr, ...) @printf(ptr @3)
  br label %for_increment

for_merge:                                        ; preds = %for_condition
  %strtmp60 = alloca [13 x i8], align 1
  store [13 x i8] c"result: z = \00", ptr %strtmp60, align 1
  %z61 = load i32, ptr %z, align 4
  %69 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp60)
  %70 = call i32 (ptr, ...) @printf(ptr @0, i32 %z61)
  %71 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp62 = alloca [11 x i8], align 1
  store [11 x i8] c"while loop\00", ptr %strtmp62, align 1
  %72 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp62)
  %73 = call i32 (ptr, ...) @printf(ptr @3)
  br label %while_condition63

while_condition63:                                ; preds = %while_stmt64, %for_merge
  %x66 = load i32, ptr %x, align 4
  %sletmp67 = icmp sle i32 %x66, 10
  br i1 %sletmp67, label %while_stmt64, label %while_done65

while_stmt64:                                     ; preds = %while_condition63
  %strtmp68 = alloca [5 x i8], align 1
  store [5 x i8] c"x = \00", ptr %strtmp68, align 1
  %x69 = load i32, ptr %x, align 4
  %74 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp68)
  %75 = call i32 (ptr, ...) @printf(ptr @0, i32 %x69)
  %76 = call i32 (ptr, ...) @printf(ptr @3)
  %loadtmp70 = load i32, ptr %x, align 4
  %addtmp71 = add i32 %loadtmp70, 1
  store i32 %addtmp71, ptr %x, align 4
  br label %while_condition63

while_done65:                                     ; preds = %while_condition63
  %strtmp72 = alloca [13 x i8], align 1
  store [13 x i8] c"result: x = \00", ptr %strtmp72, align 1
  %x73 = load i32, ptr %x, align 4
  %77 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp72)
  %78 = call i32 (ptr, ...) @printf(ptr @0, i32 %x73)
  %79 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp74 = alloca [22 x i8], align 1
  store [22 x i8] c"scoped var assignment\00", ptr %strtmp74, align 1
  %80 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp74)
  %81 = call i32 (ptr, ...) @printf(ptr @3)
  %k = alloca i32, align 4
  store i32 1, ptr %k, align 4
  %loadtmp75 = load i32, ptr %k, align 4
  br i1 true, label %true76, label %false77

true76:                                           ; preds = %while_done65
  br label %done78

false77:                                          ; preds = %while_done65
  br label %done78

done78:                                           ; preds = %false77, %true76
  %"ternary result79" = phi i32 [ 1, %true76 ], [ 2, %false77 ]
  %addtmp80 = add i32 %loadtmp75, %"ternary result79"
  store i32 %addtmp80, ptr %k, align 4
  %strtmp81 = alloca [13 x i8], align 1
  store [13 x i8] c"result: k = \00", ptr %strtmp81, align 1
  %k82 = load i32, ptr %k, align 4
  %82 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp81)
  %83 = call i32 (ptr, ...) @printf(ptr @0, i32 %k82)
  %84 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp83 = alloca [21 x i8], align 1
  store [21 x i8] c"nested-nested scopes\00", ptr %strtmp83, align 1
  %85 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp83)
  %86 = call i32 (ptr, ...) @printf(ptr @3)
  %y84 = alloca i32, align 4
  store i32 0, ptr %y84, align 4
  %loadtmp85 = load i32, ptr %y84, align 4
  %addtmp86 = add i32 %loadtmp85, 1
  store i32 %addtmp86, ptr %y84, align 4
  %y87 = load i32, ptr %y84, align 4
  %87 = call i32 (ptr, ...) @printf(ptr @0, i32 %y87)
  %88 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp88 = alloca [13 x i8], align 1
  store [13 x i8] c"result: y = \00", ptr %strtmp88, align 1
  %y89 = load i32, ptr %y84, align 4
  %89 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp88)
  %90 = call i32 (ptr, ...) @printf(ptr @0, i32 %y89)
  %91 = call i32 (ptr, ...) @printf(ptr @3)
  %j = alloca i32, align 4
  store i32 0, ptr %j, align 4
  %strtmp90 = alloca [23 x i8], align 1
  store [23 x i8] c"while continue & break\00", ptr %strtmp90, align 1
  %92 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp90)
  %93 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp91 = alloca [17 x i8], align 1
  store [17 x i8] c"j should equal 3\00", ptr %strtmp91, align 1
  %94 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp91)
  %95 = call i32 (ptr, ...) @printf(ptr @3)
  br label %while_condition92

while_condition92:                                ; preds = %unreachable102, %then99, %done78
  br i1 true, label %while_stmt93, label %while_done94

while_stmt93:                                     ; preds = %while_condition92
  %loadtmp95 = load i32, ptr %j, align 4
  %addtmp96 = add i32 %loadtmp95, 1
  store i32 %addtmp96, ptr %j, align 4
  %j97 = load i32, ptr %j, align 4
  %sletmp98 = icmp sle i32 %j97, 2
  br i1 %sletmp98, label %then99, label %if_cont100

while_done94:                                     ; preds = %if_cont100, %while_condition92
  %strtmp103 = alloca [5 x i8], align 1
  store [5 x i8] c"j = \00", ptr %strtmp103, align 1
  %j104 = load i32, ptr %j, align 4
  %96 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp103)
  %97 = call i32 (ptr, ...) @printf(ptr @0, i32 %j104)
  %98 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp105 = alloca [20 x i8], align 1
  store [20 x i8] c"while test continue\00", ptr %strtmp105, align 1
  %99 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp105)
  %100 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp106 = alloca [26 x i8], align 1
  store [26 x i8] c"shouldnt print when o = 2\00", ptr %strtmp106, align 1
  %101 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp106)
  %102 = call i32 (ptr, ...) @printf(ptr @3)
  %o = alloca i32, align 4
  store i32 0, ptr %o, align 4
  br label %while_condition107

then99:                                           ; preds = %while_stmt93
  br label %while_condition92

if_cont100:                                       ; preds = %unreachable101, %while_stmt93
  br label %while_done94

unreachable101:                                   ; No predecessors!
  br label %if_cont100

unreachable102:                                   ; No predecessors!
  br label %while_condition92

while_condition107:                               ; preds = %if_cont115, %then114, %while_done94
  %o110 = load i32, ptr %o, align 4
  %slttmp = icmp slt i32 %o110, 5
  br i1 %slttmp, label %while_stmt108, label %while_done109

while_stmt108:                                    ; preds = %while_condition107
  %loadtmp111 = load i32, ptr %o, align 4
  %addtmp112 = add i32 %loadtmp111, 1
  store i32 %addtmp112, ptr %o, align 4
  %o113 = load i32, ptr %o, align 4
  %eqtmp = icmp eq i32 %o113, 2
  br i1 %eqtmp, label %then114, label %if_cont115

while_done109:                                    ; preds = %while_condition107
  %strtmp119 = alloca [17 x i8], align 1
  store [17 x i8] c"scope assignment\00", ptr %strtmp119, align 1
  %103 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp119)
  %104 = call i32 (ptr, ...) @printf(ptr @3)
  %p = alloca i32, align 4
  store i32 0, ptr %p, align 4
  %loadtmp120 = load i32, ptr %p, align 4
  %addtmp121 = add i32 %loadtmp120, 1
  store i32 %addtmp121, ptr %p, align 4
  %p122 = load i32, ptr %p, align 4
  %105 = call i32 (ptr, ...) @printf(ptr @0, i32 %p122)
  %106 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp123 = alloca [18 x i8], align 1
  store [18 x i8] c"for loop, print y\00", ptr %strtmp123, align 1
  %107 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp123)
  %108 = call i32 (ptr, ...) @printf(ptr @3)
  br label %for_initializer124

then114:                                          ; preds = %while_stmt108
  br label %while_condition107

if_cont115:                                       ; preds = %unreachable116, %while_stmt108
  %strtmp117 = alloca [5 x i8], align 1
  store [5 x i8] c"o = \00", ptr %strtmp117, align 1
  %o118 = load i32, ptr %o, align 4
  %109 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp117)
  %110 = call i32 (ptr, ...) @printf(ptr @0, i32 %o118)
  %111 = call i32 (ptr, ...) @printf(ptr @3)
  br label %while_condition107

unreachable116:                                   ; No predecessors!
  br label %if_cont115

for_initializer124:                               ; preds = %while_done109
  %y129 = alloca i32, align 4
  store i32 0, ptr %y129, align 4
  br label %for_condition125

for_condition125:                                 ; preds = %for_increment126, %for_initializer124
  %y130 = load i32, ptr %y129, align 4
  %sletmp131 = icmp sle i32 %y130, 5
  br i1 %sletmp131, label %for_body127, label %for_merge128

for_increment126:                                 ; preds = %for_body127
  %loadtmp135 = load i32, ptr %y129, align 4
  %addtmp136 = add i32 %loadtmp135, 1
  store i32 %addtmp136, ptr %y129, align 4
  br label %for_condition125

for_body127:                                      ; preds = %for_condition125
  %y132 = load i32, ptr %y129, align 4
  %112 = call i32 (ptr, ...) @printf(ptr @0, i32 %y132)
  %113 = call i32 (ptr, ...) @printf(ptr @3)
  %loadtmp133 = load i32, ptr %p, align 4
  %y134 = load i32, ptr %y129, align 4
  store i32 %y134, ptr %p, align 4
  br label %for_increment126

for_merge128:                                     ; preds = %for_condition125
  %strtmp137 = alloca [23 x i8], align 1
  store [23 x i8] c"assignment in for loop\00", ptr %strtmp137, align 1
  %114 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp137)
  %115 = call i32 (ptr, ...) @printf(ptr @3)
  %p138 = load i32, ptr %p, align 4
  %116 = call i32 (ptr, ...) @printf(ptr @0, i32 %p138)
  %117 = call i32 (ptr, ...) @printf(ptr @3)
  %strtmp139 = alloca [36 x i8], align 1
  store [36 x i8] c"for-loop in nested scope assignment\00", ptr %strtmp139, align 1
  %118 = call i32 (ptr, ...) @printf(ptr @2, ptr %strtmp139)
  %119 = call i32 (ptr, ...) @printf(ptr @3)
  %q = alloca i32, align 4
  store i32 0, ptr %q, align 4
  br label %for_initializer140

for_initializer140:                               ; preds = %for_merge128
  %y145 = alloca i32, align 4
  store i32 0, ptr %y145, align 4
  br label %for_condition141

for_condition141:                                 ; preds = %for_increment142, %for_initializer140
  %y146 = load i32, ptr %y145, align 4
  %sletmp147 = icmp sle i32 %y146, 5
  br i1 %sletmp147, label %for_body143, label %for_merge144

for_increment142:                                 ; preds = %for_body143
  %loadtmp150 = load i32, ptr %y145, align 4
  %addtmp151 = add i32 %loadtmp150, 1
  store i32 %addtmp151, ptr %y145, align 4
  br label %for_condition141

for_body143:                                      ; preds = %for_condition141
  %loadtmp148 = load i32, ptr %q, align 4
  %y149 = load i32, ptr %y145, align 4
  store i32 %y149, ptr %q, align 4
  br label %for_increment142

for_merge144:                                     ; preds = %for_condition141
  %q152 = load i32, ptr %q, align 4
  %120 = call i32 (ptr, ...) @printf(ptr @0, i32 %q152)
  %121 = call i32 (ptr, ...) @printf(ptr @3)
  %r = alloca double, align 8
  store double 5.000000e+00, ptr %r, align 8
  %loadtmp153 = load double, ptr %r, align 8
  %fdivtmp = fdiv double %loadtmp153, 2.000000e+00
  store double %fdivtmp, ptr %r, align 8
  %r154 = load double, ptr %r, align 8
  %122 = call i32 (ptr, ...) @printf(ptr @1, double %r154)
  %123 = call i32 (ptr, ...) @printf(ptr @3)
  ret void
}

define void @printDoubleI(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  %x2 = load i32, ptr %x1, align 4
  %smultmp = mul i32 %x2, 2
  %0 = call i32 (ptr, ...) @printf(ptr @0, i32 %smultmp)
  %1 = call i32 (ptr, ...) @printf(ptr @3)
  ret void
}

define void @printDoubleF(double %x) {
entry:
  %x1 = alloca double, align 8
  store double %x, ptr %x1, align 8
  %x2 = load double, ptr %x1, align 8
  %fmultmp = fmul double %x2, 2.000000e+00
  %0 = call i32 (ptr, ...) @printf(ptr @1, double %fmultmp)
  %1 = call i32 (ptr, ...) @printf(ptr @3)
  ret void
}

define i32 @double(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  %x2 = load i32, ptr %x1, align 4
  %smultmp = mul i32 %x2, 2
  ret i32 %smultmp
}

define i32 @quadruple(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  %x2 = load i32, ptr %x1, align 4
  %calltmp = call i32 @double(i32 %x2)
  %smultmp = mul i32 %calltmp, 2
  ret i32 %smultmp
}
