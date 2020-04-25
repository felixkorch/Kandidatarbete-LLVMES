path_to_jit = "."

test_list = {
    "adc",
    "jmp_idr_test",
    "cpy_abs",
    "jsr_test",
    "sbc"
}

number_of_tests_successful = 0

local function bool_to_number(value)
    return value and 1 or 0
end

for i, v in ipairs(test_list) do
    local a1, a2, a3 = os.execute("./list-bins/jit -O list-bins/" .. v .. ".bin")
    if a3 ~= 0 then
        print(v .. " failed with exit code " .. tostring(bool_to_number(a3)))
        io.read()
    else
        number_of_tests_successful = number_of_tests_successful + 1
    end
end

print("----------------------------------------------")
print(number_of_tests_successful .. " / " .. #test_list .. " passed")
