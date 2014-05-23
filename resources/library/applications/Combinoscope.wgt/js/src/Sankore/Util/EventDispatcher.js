/*globals klass: true, Sankore:true*/
(function () {
    "use strict";

    klass.define('Sankore.Util', 'EventDispatcher', klass.extend({
        constructor: function () {
            this.eventListeners = {};
        },

        addEventSubscriber: function (subscriber) {
            for (var i in subscriber.events) {
                this.addEventListener(subscriber.events[i], subscriber.listener);
            }

            return this;
        },

        addEventListener: function (event, closure, id) {
            if (typeof this.eventListeners[event] === 'undefined') {
                this.eventListeners[event] = [];
            }

            if (typeof id === 'undefined') {
                this.eventListeners[event].push(closure);
            } else {
                this.eventListeners[event][id] = closure;
            }

            return this;
        },

        removeEventListener: function (event, id) {
            delete this.eventListeners[event][id];
        },

        removeAllEventListeners: function (event) {
            this.eventListeners[event] = [];
        },

        notify: function (event, obj) {
            var closure,
                handlers = this.eventListeners[event] || [],
                self = this;

            Object.keys(this.eventListeners).map(function (name) {
                if (name.indexOf('*') !== -1 && name !== event) {
                    var regexp = new RegExp(name.replace('*', '(.*)'));

                    if (regexp.test(event)) {
                        handlers = handlers.concat(self.eventListeners[name]);
                    }
                }
            });

            for (closure in handlers) {
                handlers[closure](obj);
            }
        }
    }));
})();