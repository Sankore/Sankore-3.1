/* globals klass:true, Sankore:true */
(function () {
    "use strict";
    
    klass.define('Sankore', 'Layout', klass.extend({
        constructor: function (name, editable) {
            this.name = name;
            this.editable = typeof editable !== 'undefined' ? editable : true;
            this.letters = [];
        },
        
        /**
         * @param {Array} letters
         */
        setLetters: function (letters) {
            this.letters = letters;
        },
        
        /**
         * @param {String} letter
         */
        addLetter: function (letter) {
            if (!this.containLetter(letter)) {
                this.letters.push(letter);
            }
        },

        /**
         * @param {String} letter
         * @returns {Boolean}
         */
        containLetter: function (letter) {
            return this.letters.indexOf(letter) !== -1;
        },
        
        /**
         * @param {String} Letter
         */
        removeLetter: function (letter) {
            if (this.containLetter(letter)) {
                this.letters.splice(this.letters.indexOf(letter), 1);
            }
        }
    }));
})();