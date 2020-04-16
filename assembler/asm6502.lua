-- Cache library functions.
local type, pairs, ipairs = type, pairs, ipairs
local pcall, error, assert = pcall, error, assert
local _s = string
local sub, match, gmatch, gsub = _s.sub, _s.match, _s.gmatch, _s.gsub
local find, format, rep, upper = _s.format, _s.rep, _s.upper, _s.find
local _t = table
local insert, remove, concat, sort = _t.insert, _t.remove, _t.concat, _t.sort
local exit = os.exit
local io = io
local stdin, stdout, stderr = io.stdin, io.stdout, io.stderr

local function write_bytes(to, array)
    local out = assert(io.open(to, "wb"))
    for _, x in ipairs(array) do
        local b = string.char(x % 256)
        out:write(b)
    end
    assert(out:close())
end

local function str_to_bytes(str)
    assert(str ~= nil, "Can't convert nil to number")
    assert(type(str) == "string", "Can't convert non-string to number")

    if str:sub(1, 1) == "$" then
        str = str:sub(2)
    else
        return tonumber(str), nil
    end

    local len = string.len(str)

    if len == 1 or len == 2 then
        return tonumber("0x"..str), nil
    elseif len == 4 then
        local b1 = str:sub(1, 2)
        local b2 = str:sub(3, 4)
        return tonumber("0x"..b1), tonumber("0x"..b2)
    else
        error("String doesn't represent a number")
        return 0, 0
    end
end

local function str_to_number(str)
    assert(str ~= nil, "Can't convert nil to number")
    assert(type(str) == "string", "Can't convert non-string to number")

    if str:sub(1, 1) == "$" then
        return tonumber("0x"..str:sub(2))
    else
        return tonumber(str)
    end
end

local function number_to_str(num)
    assert(num ~= nil, "Can't convert nil to string")
    assert(type(num) == "number", "Can't convert non-number to string")

    return string.format("$%x", num)
end

local addressing_mode_size = {
    IMM = 2, ZP = 2, ZPX = 2, ABS = 3,
    ABSX = 3, ABSY = 3, INDX = 2, INDY = 2, IND = 3,
    REL = 2, ACC = 1, IMP = 1
}

local map_op = {
    LDA = { IMM = "A9", ZP = "A5", ZPX = "B5", ABS = "AD", ABSX = "BD", ABSY = "B9", INDX = "A1", INDY = "B1" },
    LDY = { IMM = "A0", ZP = "A4", ZPX = "B4", ABS = "AC", ABSX = "BC" },
    CPY = { IMM = "C0", ZP = "C4", ABS = "CC" },
    DEX = { IMP = "CA" },
    DEY = { IMP = "88" },
    INX = { IMP = "E8" },
    STA = { ZP = "85", ZPX = "95", ABS = "8D", ABSX = "9D", ABSY = "99", INDX = "81", INDY = "91" },
    JMP = { ABS = "4C", IND = "6C" },
    BNE = { REL = "D0" },
    BEQ = { REL = "F0" },
    TYA = { IMP = "98" },
    TAY = { IMP = "A8" }
}

local label_regex = "^([a-z]+):$"
local org_regex = ".org"
local db_regex = ".db"
local dw_regex = ".dw"
local hex_value_regex = "^%$?%w+$"
local immediate_value_regex = "^#(%$?%x+)$"
local name_regex = "^[a-z]+$"
local register_regex = "^[XY]$"
local indirect_regex = "^%((%$%x+)%)$"

local tokens = {}
local labels = {}
local machine_code = {}
local scan_line = 1

local valid_successors = {
    ["Instruction"] = {
        ["Value"] = true,
        ["Name"] = true,
        ["ImmediateValue"] = true,
        ["IndirectArgument"] = true,
        ["Instruction"] = true
    }
    ;
    ["Label"] = {
        ["Instruction"] = true,
        ["DataByte"] = true,
        ["DataWord"] = true
    }
    ;
    ["Comma"] = {
        ["Register"] = true,
        ["Value"] = true
    }
    ;
    ["Org"] = {
        ["Value"] = true
    }
    ;
    ["DataByte"] = {
        ["Value"] = true,
        ["String"] = true
    }
    ;
    ["DataWord"] = {
        ["Value"] = true,
        ["Name"] = true
    }
}

local valid_predecessor = {
    ["Name"] = {
        ["Instruction"] = true,
        ["DataWord"] = true
    }
}

local function check_ordering(stmt, pred, successor)
    if pred ~= nil then
        if valid_predecessor[stmt.type] ~= nil then
            if valid_predecessor[stmt.type][pred.type] == nil then
                error("Error: "..stmt.type.." invalid predecessor: '"..pred.type.."' at line "..stmt.line)
            end
        end
    end
    if successor ~= nil then
        if valid_successors[stmt.type] == nil then
            return
        end
        if valid_successors[stmt.type][successor.type] == nil then
            error("Error: "..stmt.type.." invalid successor: '"..successor.type.."' at line "..stmt.line)
        end
    end
end

local function get_addressing_mode(tokens, i)

    local addressing_mode

    local instruction = tokens[i]
    local token1 = tokens[i + 1]
    local token2 = tokens[i + 2]
    local token3 = tokens[i + 3]

    if token1 == nil then token1 = {}; token1.type = "" end
    if token2 == nil then token2 = {}; token2.type = "" end
    if token3 == nil then token3 = {}; token3.type = "" end

    if token1.type == "Value" and token2.type == "Comma" and token3.type == "Register" then
        local base_mode = str_to_number(token1.data) <= 255 and "ZP" or "ABS"
        addressing_mode = base_mode..token3.data
    elseif token1.type == "Name" and token2.type == "Comma" and token3.type == "Register" then
        addressing_mode = "ABS"..token3.data
    elseif token1.type == "IndirectArgument" and token2.type == "Comma" and token3.type == "Register" then
        addressing_mode = "IND"..token3.data
    end

    if addressing_mode then return addressing_mode end

    if token1.type == "ImmediateValue" then
        addressing_mode = "IMM"
    elseif token1.type == "Value" then
        addressing_mode = str_to_number(token1.data) <= 255 and "ZP" or "ABS"
    elseif token1.type == "IndirectArgument" then
        addressing_mode = "IND"
    elseif token1.type == "Name" then
        addressing_mode = instruction.data == "JMP" and "ABS" or "REL"
    else
        addressing_mode = "IMP"
    end

    return addressing_mode
    
end

local function parse(tokens)

    local lc = 0

    -- This is syntax checking basically
    for i, v in ipairs(tokens) do
        check_ordering(v, tokens[i - 1], tokens[i + 1])
    end

    local first_token = tokens[1]
    assert(first_token, "Error: Empty file")
    assert(first_token.type == "Org", "First statement in the program must be '.org'")
    lc = str_to_number(tokens[2].data)

    for i = 3, #tokens do -- First pass, resolve addresses

        local v = tokens[i]

        if v.type == "Org" then

            local val = tokens[i + 1]
            v.start = lc
            lc = str_to_number(val.data)

        elseif v.type == "Label" then

            labels[v.data] = { address = number_to_str(lc) }

        elseif v.type == "DataByte" then

            local j, w = next(tokens, i)
            while j do
                if w.type == "String" then
                    lc = lc + #w.data
                elseif w.type == "Value" then
                    lc = str_to_number(w.data) <= 255 and lc + 1 or lc + 2
                elseif w.type == "Comma" then
                    -- Skip
                else
                    break
                end
                j, w = next(tokens, j)
            end

        elseif v.type == "DataWord" then

            lc = lc + 2

        elseif v.type == "Instruction" then

            local addressing_mode = get_addressing_mode(tokens, i)
            local instruction_size = addressing_mode_size[addressing_mode]
            lc = lc + instruction_size
            v.mode = addressing_mode
            v.address = lc
            v.size = instruction_size
        end
    end

    for i, v in ipairs(tokens) do -- Second pass, generate machine code

        if v.type == "Instruction" then

            local opcode = map_op[v.data][v.mode]
            if opcode == nil then
                error("Opcode doesn't exist")
            end

            local arg = tokens[i + 1]
            local arg_address

            if arg then
                if arg.type == "Name" then
                    local label = labels[arg.data]
                    assert(label, "Undefined reference to label '"..arg.data.."'")
                    arg_address = label.address
                elseif arg.type == "Value" or arg.type == "ImmediateValue" then
                    arg_address = arg.data
                end
            end

            machine_code[#machine_code + 1] = tonumber("0x"..opcode)

            local switch = {
                ["REL"] = function()
                    local diff = str_to_number(arg_address) - v.address
                    machine_code[#machine_code + 1] = diff
                end,
                ["IMP"] = function()
                    -- Do nothing
                end,
                ["Default"] = function ()
                    local b2, b1 = str_to_bytes(arg_address) -- Little endian, so swap order
                    if b1 ~= nil then machine_code[#machine_code + 1] = b1 end
                    if b2 ~= nil then machine_code[#machine_code + 1] = b2 end
                end
            }

            local s = switch[v.mode]
            if s == nil then s = switch["Default"] end
            s()

        elseif v.type == "DataByte" then

            local j, w = next(tokens, i)
            while j do
                if w.type == "String" then
                    for q = 1, #w.data do
                        local c = w.data:sub(q, q)
                        machine_code[#machine_code + 1] = string.byte(c)
                    end
                elseif w.type == "Value" then
                    machine_code[#machine_code + 1] = str_to_bytes(w.data)
                elseif w.type == "Comma" then
                    -- Skip
                else
                    break
                end
                j, w = next(tokens, j)
            end

        elseif v.type == "DataWord" then

            local arg = tokens[i + 1]
            local b1, b2

            if arg.type == "Name" then
                local label = labels[arg.data]
                assert(label, "Undefined reference to label '"..arg.data.."'")
                b1, b2 = str_to_bytes(label.address)
            elseif arg.type == "Value" or arg.type == "ImmediateValue" then
                b1, b2 = str_to_bytes(arg.data)
            end

            if b1 ~= nil then machine_code[#machine_code + 1] = b1 end
            if b2 ~= nil then machine_code[#machine_code + 1] = b2 end

        elseif v.type == "Org" then

            if v.start ~= nil then
                local val = tokens[i + 1]
                local start = v.start
                local stop = str_to_number(val.data)

                for z = start, stop - 1 do
                    machine_code[#machine_code + 1] = 0
                end
            end

        end
        
    end
end

local function parse_symbol(line, pos)

    local c = line:sub(pos, pos)

    local symbol = ""

    while true do
        if c == " " or c == ";" or c == "" or c == "," then
            break
        end

        if string.byte(c) == 13 then -- Weird character on Windows, skip it
            c = line:sub(pos + 1, pos + 1)
        end

        symbol = symbol..c

        pos = pos + 1
        c = line:sub(pos, pos)
    end

    if map_op[symbol] then
        tokens[#tokens + 1] = { data = symbol, type = "Instruction", line = scan_line }
    elseif symbol:match(label_regex) then
        tokens[#tokens + 1] = { data = symbol:sub(1, -2), type = "Label", line = scan_line } -- TODO: Fix this
    elseif symbol:match(org_regex) then
        tokens[#tokens + 1] = { data = symbol, type = "Org", line = scan_line }
    elseif symbol:match(db_regex) then
        tokens[#tokens + 1] = { data = symbol, type = "DataByte", line = scan_line }
    elseif symbol:match(dw_regex) then
        tokens[#tokens + 1] = { data = symbol, type = "DataWord", line = scan_line }
    elseif symbol:match(immediate_value_regex) then
        tokens[#tokens + 1] = { data = symbol:match(immediate_value_regex), type = "ImmediateValue", line = scan_line } -- TODO: Fix this
    elseif symbol:match(name_regex) then
        tokens[#tokens + 1] = { data = symbol, type = "Name", line = scan_line }
    elseif symbol:match(register_regex) then
        tokens[#tokens + 1] = { data = symbol, type = "Register", line = scan_line }
    elseif symbol:match(hex_value_regex) then
        tokens[#tokens + 1] = { data = symbol, type = "Value", line = scan_line }
    elseif symbol:match(indirect_regex) then
        tokens[#tokens + 1] = { data = symbol:match(indirect_regex), type = "IndirectArgument", line = scan_line } -- TODO: Fix this
    elseif symbol == "" then
    else
        error("Error: Unknown symbol '"..symbol.."' at line "..scan_line)
    end

    return symbol, pos - 1
end

local function parse_string(line, pos)

    pos = pos + 1
    local c = line:sub(pos, pos)
    local symbol = ""

    while true do
        if c == "\"" or c == "" then
            break
        end

        symbol = symbol..c

        pos = pos + 1
        c = line:sub(pos, pos)
    end

    tokens[#tokens + 1 ] = { data = symbol, type = "String", line = scan_line }

    return symbol, pos
end

local function lex_line(line)

    local c = line:sub(1, 1)
    local i = 1

    while true do

        if c == "" then
            break
        end

        if c == " " then
            -- Do Nothing
        elseif c == ";" then
            break
        elseif c == "\"" then
            local symb, pos = parse_string(line, i)
            i = pos
        elseif c == "," then
            tokens[#tokens + 1] = { data = ",", type = "Comma", line = scan_line }
        else
            local symb, pos = parse_symbol(line, i)
            i = pos
        end

        i = i + 1
        c = line:sub(i, i)
    end
end

local function lex(str)
    str = str.."\n"
    local newline = string.byte("\n")
    local prev = 1
    for i = 1, #str do
        local c = str:byte(i)
        if c == newline then
            lex_line(str:sub(prev, i - 1))
            prev = i + 1
            scan_line = scan_line + 1
        end
    end
end


local function read_file_to_string(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return content
end

local function parse_args(args)

    if #args ~= 1 then
        error("Only one argument allowed")
    end

    local list_file = args[1]

    if list_file:sub(-3) ~= "lst" then
        error("Only .lst files allowed")
    end

    local content = read_file_to_string(list_file)

    lex(content)
    parse(tokens)
    write_bytes(list_file:sub(1, -4).."bin", machine_code)
end

local execution_time = os.clock()
parse_args{...}
execution_time = os.clock() - execution_time

for i, v in ipairs(tokens) do
    print(v.data, "", v.type)
end

print("\nTime elapsed: "..execution_time.."s")