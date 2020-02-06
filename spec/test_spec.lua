describe("tests", function () 
   local lvoikko = require("lvoikko")

   local voikko = lvoikko.new("fi_FI")
   
   it("loads succesfully", function ()
      assert.is_table(voikko)
   end)

   describe("get_hyphenation_pattern", function ()

      it("should give the correct pattern of the word \"kukkaruukku\"", function ()
         local word = voikko:get_hyphenation_pattern("kukkaruukku")
         assert.are.equals("   - -   - ", word)
      end)

      it("should give the correct pattern of the word \"mitääntekemättömyyksillänsäkään\"", function ()
         local word = voikko:get_hyphenation_pattern("mitääntekemättömyyksillänsäkään")
         assert.are.equals("  -   - - -  - -   -  -  - -   ", word)
      end)

      it("should give the correct pattern of the word \"sika-nautajauheliha\"", function ()
         local word = voikko:get_hyphenation_pattern("sika-nautajauheliha")
         assert.are.equals("  - =   - -  - - - ", word)
      end)

      it("should give string full of spaces", function ()
         local word = voikko:get_hyphenation_pattern("asdfghjkl")
         assert.are.equals("         ", word)
      end)

      it("should give empty string", function ()
         local word = voikko:get_hyphenation_pattern("")
         assert.are.equals("", word)
      end)

   end)

   describe("hyphenate", function ()

      it("should hyphenate correctly the word \"kukkaruukku\"", function ()
         local word = voikko:hyphenate("kukkaruukku")
         assert.are.equals("kuk-ka-ruuk-ku", word)
      end)

      it("should hyphenate correctly the word \"oleskeluyhteiskunta\" and use \"_\" as a hyphen", function ()
         local word = voikko:hyphenate("oleskeluyhteiskunta", "_")
         assert.are.equals("o_les_ke_lu_yh_teis_kun_ta", word)
      end)

      it("should hyphenate correctly the word \"sillisalaatti\" and use \"\\-\" as a hyphen", function ()
         local word = voikko:hyphenate("sillisalaatti", "\\-")
         assert.are.equals("sil\\-li\\-sa\\-laat\\-ti", word)
      end)

      it("should allow context change when hyphenating the word \"vaa'an\" using \"&shy;\" as a hyphen", function ()
         local word = voikko:hyphenate("vaa'an", "&shy;", true)
         assert.are.equals("vaa&shy;an", word)
      end)

      it("should not allow context change when hyphenating the word \"vaa'an\" using \"&shy;\" as a hyphen", function ()
         local word = voikko:hyphenate("vaa'an", "&shy;", false)
         assert.are.equals("vaa'an", word)
      end)

   end)

   describe("spell", function ()

      it("should give return 1 (spell ok)", function ()
         local code = voikko:spell("sydämen")
         assert.are.equals(1, code)
      end)

      it("should give return 0 (spell failed)", function ()
         local code = voikko:spell("koiro")
         assert.are.equals(0, code)
      end)

      it("should give return 0 (spell failed)", function ()
         local code = voikko:spell("asdfghjkl")
         assert.are.equals(0, code)
      end)

      it("should give return 0 (spell failed)", function ()
         local code = voikko:spell("αβγδε")
         assert.are.equals(0, code)
      end)

      it("should give return 1 (spell ok)", function ()
         local code = voikko:spell("oleskeluyhteiskunta")
         assert.are.equals(1, code)
      end)

   end)

   describe("suggest", function ()

      it("should give suggestions { \"sydämen\" }", function ()
         local suggestions = voikko:suggest("sydämmen")
         assert.are.same({ "sydämen" }, suggestions)
      end)

      it("should give suggestions { \"vielä\", \"vikellä\", \"vipellä\", \"vihellä\", \"Villeä\" }", function ()
         local suggestions = voikko:suggest("viellä")
         assert.are.same({ "vielä", "vikellä", "vipellä", "vihellä", "Villeä" }, suggestions)
      end)

      it("should give suggestions { \"jauheliha\", \"jauheenliha\", \"jouhenliha\" }", function ()
         local suggestions = voikko:suggest("jauhenliha")
         assert.are.same({ "jauheliha", "jauheenliha", "jouhenliha" }, suggestions)
      end)

      it("should give suggestions { \"kuulostaa\", \"kuullosta\", \"kuullostaan\", \"kuullostapa\" }", function ()
         local suggestions = voikko:suggest("kuullostaa")
         assert.are.same({ "kuulostaa", "kuullosta", "kuullostaan", "kuullostapa" }, suggestions)
      end)

      it("should give no suggestions", function ()
         local suggestions = voikko:suggest("asdfghjkl")
         assert.is_nil(suggestions)
      end)

   end)

end)
