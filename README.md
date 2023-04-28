# Dynamisk omkompilering utav MOS 6502-maskinkod, genom LLVM IR med hjälp av JIT-kompilering

## Beskrivning
Projektet undersöker möjligheten att implementera dynamisk omkompilering av maskinkod menad för processorn MOS 6502 med hjälp av LLVM. Genom att kombinera LLVMs bibliotek för kompilering och optimering utvecklades en emulator för processorn MOS 6502. Dessutom utvecklades en interpreterande emulator, vilken används som utgångspunkt för att jämföra och analysera exekveringshastigheten av maskinkoden genererad av den dynamiskt omkompilerande emulatorn. Exekveringen av programmen som testades på de två emulatorerna utfördes alltid snabbare på den dynamiskt omkompilerande emulatorn jämfört med den interpreterande emulatorn. Den dynamiskt omkompilerande emulatorn har en uppstartningsfas som i vissa fall leder till att den interpreterade emulatorn överlag slutför exekveringen snabbare.


## Gruppmedlemmar
- Carl Blomqvist (D)
- Matilda Falkenby (IT)
- William Kalin (D)
- Felix Korch (D)
- Carl Lundgren (D)
- Markus Pettersson (IT)