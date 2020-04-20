; ModuleID = 'loops.bin'
source_filename = "loops.bin"

declare void @putreg(i8)

declare void @putchar(i8)

define i32 @main() {
entry:
  %X = alloca i8, align 1
  %Y = alloca i8, align 1
  %V = alloca i1, align 1
  store i8 0, i8* %X, align 1
  store i8 0, i8* %Y, align 1
  store i1 false, i1* %V, align 1
  store i8 -1, i8* %Y, align 1
  br label %"Label $8002"

"Label $8002":                                    ; preds = %"AutoLabel 0", %entry
  %0 = phi i8 [ %3, %"AutoLabel 0" ], [ -1, %entry ]
  br label %"Label $8004"

"Label $8004":                                    ; preds = %"Label $8004", %"Label $8002"
  %1 = phi i8 [ %2, %"Label $8004" ], [ -1, %"Label $8002" ]
  %2 = add i8 %1, -1
  %eq = icmp eq i8 %2, 0
  br i1 %eq, label %"AutoLabel 0", label %"Label $8004"

"AutoLabel 0":                                    ; preds = %"Label $8004"
  %3 = add i8 %0, -1
  %eq1 = icmp eq i8 %3, 0
  br i1 %eq1, label %"AutoLabel 1", label %"Label $8002"

"AutoLabel 1":                                    ; preds = %"AutoLabel 0"
  store i8 %2, i8* %X, align 1
  store i8 %3, i8* %Y, align 1
  call void @putreg(i8 0)
  ret i32 0
}

declare void @write(i16, i8)

declare i8 @read(i16)
