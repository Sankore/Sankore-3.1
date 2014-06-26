/* global klass:true, Sankore:true */
(function () {
    "use strict";

    klass.define('Sankore', 'Letter', klass.extend({
        constructor: function (name, type) {
            this.name = name.toLowerCase();

            if (type !== 'C' && type !== 'V') {
                throw new Error('Letter must be type C or V');
            }

            this.type = type;
            this.id = this.type + this.name;
        }
    }));
})();