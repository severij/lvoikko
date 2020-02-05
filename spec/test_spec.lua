describe("tests", function () 
   local lvoikko = require("lvoikko")

   local voikko = lvoikko.init("fi_FI")
   
   it("loads succesfully", function ()
      assert.is_table(voikko)
   end)

   describe("hyphenate", function ()

      it("should hyphenate correctly the word \"kukkaruukku\"", function ()
         local word = voikko.hyphenate("kukkaruukku")
         assert.are.equals(word, "   - -   - ")
      end)

      it("should hyphenate correctly the word \"mitääntekemättömyyksillänsäkään\"", function ()
         local word = voikko.hyphenate("mitääntekemättömyyksillänsäkään")
         assert.are.equals(word, "  -   - - -  - -   -  -  - -   ")
      end)

      it("should hyphenate correctly the word \"lentokonesuihkuturbiinimoottoriapumekaanikko\"", function ()
         local word = voikko.hyphenate("lentokonesuihkuturbiinimoottoriapumekaanikko")
         assert.are.equals(word, "   - - - -   - -  -  - -   - - -- - -  -  - ")
      end)
   end)


end)
