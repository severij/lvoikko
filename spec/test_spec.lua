describe("tests", function () 
   local lvoikko = require("lvoikko")

   local voikko = lvoikko.init("fi_FI")
   
   it("loads succesfully", function ()
      assert.is_table(voikko)
   end)

   describe("hyphenate", function ()

      it("should hyphenate correctly the word \"kukkaruukku\"", function ()
         local word = voikko.hyphenate("kukkaruukku")
         assert.are.equals("   - -   - ", word)
      end)

      it("should hyphenate correctly the word \"mitääntekemättömyyksillänsäkään\"", function ()
         local word = voikko.hyphenate("mitääntekemättömyyksillänsäkään")
         assert.are.equals("  -   - - -  - -   -  -  - -   ", word)
      end)

      it("should hyphenate correctly the word \"sika-nautajauheliha\"", function ()
         local word = voikko.hyphenate("sika-nautajauheliha")
         assert.are.equals("  - =   - -  - - - ", word)
      end)

      it("should give whitespaces string", function ()
         local word = voikko.hyphenate("asdfghjkl")
         assert.are.equals("         ", word)
      end)

      it("should give empty string", function ()
         local word = voikko.hyphenate("")
         assert.are.equals("", word)
      end)

   end)
   end)


end)
