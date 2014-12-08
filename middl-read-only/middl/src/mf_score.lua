
g=lpeg

p_SP       = g.S(" \n\t")
p_nonSP    = (g.P(1) - p_SP)
p_string   = (g.P("'")  * ( (g.P("\\") * g.P(1)) + (g.P(1) - g.P("'")))^0  * g.P("'")) +
             (g.P("\"") * ( (g.P("\\") * g.P(1)) + (g.P(1) - g.P("\"")))^0 * g.P("\""))

p_number   = g.R("09")^1

p_info_cmd = g.P("auth") + g.P("meter") + g.P("title") + g.P("key")

p_value    = g.P(":") * (p_number + p_string + p_nonSP^1)

p_info     = p_SP^0 * g.P("@") * p_info_cmd * (p_value)^0


p_header = p_info^0

p_track  = p_SP^0 * g.P("|") * p_number^0 * p_SP^1 * g.P("body")

p_body   = p_track^0

p_score  = p_header * p_body


-- -------------------------------------------------------------

test = [[
@key:Em:har
| body
|2 body
]]

test = [[
@key:Em:har
| body
|2 body
]]

print(lpeg.match(p_score,test))

print(lpeg.match(p_string, "'ab\\'csss'"))
print(lpeg.match(p_string, '"ab\\"csss"'))
