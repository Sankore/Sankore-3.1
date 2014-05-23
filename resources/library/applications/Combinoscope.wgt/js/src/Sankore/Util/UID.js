/* jshint browser:true */
/* globals klass:true Sankore:true */
(function() {
    "use strict";
    
    klass.define('Sankore.Util', 'UID', {
        generate: function () {
            var values = '',
                date = new Date(),
                id = 0,
                i;

            for (i = 0; i < document.URL.length; i++) {
                values += String(document.URL.charCodeAt(i) * (
                    date.getMilliseconds() + 
                    date.getSeconds() + 
                    date.getMinutes() + 
                    Math.round(Math.random() * 1000)
                ));
            }

            values = values.match(/.{1,10}/g);

            for (i in values) {
                id += Number(values[i]);
            }

            return id.toString(36);
        }
    });
})();