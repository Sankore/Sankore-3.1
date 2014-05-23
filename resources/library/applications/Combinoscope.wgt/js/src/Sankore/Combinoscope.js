/* globals klass:true, Sankore:true */
(function () {
    "use strict";

    klass.define('Sankore', 'Combinoscope', klass.extend({
        constructor: function (id, options) {
            this.letters = Sankore.Util.Hash.create();
            this.layouts = Sankore.Util.Hash.create();

            this.eventDispatcher = Sankore.Util.EventDispatcher.create();
            this.ui = Sankore.UI.Interface.create(id, this);

            this.word = [];
            this.layout = null;
            this.editor = false;
            this.threeBlocks = false;
            
            if (typeof options.ready === 'function') {
                this.eventDispatcher.addEventListener('combinoscope.create', options.ready.bind(this));
            }

            this.attachEventHandlers();
            
            this.eventDispatcher.notify('combinoscope.create');
        },

        /**
         *
         */
        attachEventHandlers: function () {
            var ed = this.eventDispatcher,
                self = this;

            ed.addEventListener('ui.mode_changed', function (event) {
                self.setThreeBlocksMode(event.mode);
            });

            ed.addEventListener('ui.layout_selected', function (event) {
                self.setCurrentLayout(event.layout);
            });

            ed.addEventListener('ui.clear_clicked', function (event) {
                self.clearWord();
            });

            ed.addEventListener('ui.letter_dropped', function (event) {
                self.addToWord(event.letterId, event.before);
            });

            ed.addEventListener('ui.letter_removed', function (event) {
                self.removeLetterFromWord(event.before);
            });

            ed.addEventListener('ui.editor_clicked', function () {
                if (self.editor) {
                    self.disableEditor();
                } else {
                    self.enableEditor();
                }
            });

            ed.addEventListener('ui.letter_clicked', function (event) {
                if (event.active) {
                    self.unselectLetter(event.letterId);
                } else {
                    self.selectLetter(event.letterId);
                }
            });

            ed.addEventListener('ui.add_layout_clicked', function (event) {
                self.setCurrentLayout(self.duplicateLayout());
            });

            ed.addEventListener('ui.delete_layout_clicked', function (event) {
                self.deleteLayout();

                self.setCurrentLayout('default');
            });

            ed.addEventListener('ui.layout_name_changed', function (event) {
                self.renameLayout(event.name);
            });
            
            ed.addEventListener('ui.select_all_layout_clicked', function (event) {
                self.selectAllLetters();
            });
            
            ed.addEventListener('ui.unselect_all_layout_clicked', function (event) {
                self.unselectAllLetters();
            });
        },

        /**
         * @param {Array} layouts and letters data
         */
        init: function (data, state) {
            var id, letter, layout, self = this,
                defaultLayout = Sankore.Layout.create('Tout', false);

            this.layouts.set('default', defaultLayout);
            
            if ('layouts' in data) {
                Object.keys(data.layouts).forEach(function (id) {
                    self.layouts.set(id, Sankore.Layout.create(data.layouts[id], false));
                });
            }

            if ('letters' in data && data.letters.length > 0) {
                data.letters.forEach(function (item) {
                    // index 0 : name, index 1 : type (C|V)
                    letter = Sankore.Letter.create(item[1], item[0]);
                    self.letters.set(letter.id, letter);

                    defaultLayout.addLetter(letter.id);

                    // loop through layouts (index 2)
                    item[2].forEach(function (id) {
                        layout = self.layouts.get(id);

                        if (layout) {
                            layout.addLetter(letter.id);
                        }
                    });
                });
            }
            
            if ('layouts' in state) {
                Object.keys(state.layouts).forEach(function (id) {
                    var layout = Sankore.Layout.create(state.layouts[id].name, true);
                    layout.letters = state.layouts[id].letters;
                    
                    self.layouts.set(id, layout);
                });
            }

            this.layout = 'current' in state ? state.current : 'default';
            
            if ('mode' in state) {
                this.threeBlocks = !! state.mode;
            }

            this.ui.render();
            
            if ('word' in state) {
                this.word = state.word;

                this.notifyWordChanged();
            }
            
            if ('editor' in state && state.editor) {
                this.enableEditor();
            }

            this.eventDispatcher.notify('combinoscope.init', {
                combinoscope: this
            });
        },

        /**
         *
         */
        getState: function () {
            var layouts = {};

            this.layouts.map(function (id, layout) {
                if (layout.editable) {
                    layouts[id] = layout;
                }
            });

            return {
                mode: this.threeBlocks,
                layouts: layouts,
                current: this.layout,
                editor: this.editor,
                word: this.word
            };
        },

        /**
         *
         */
        enableEditor: function () {
            this.editor = true;

            this.clearWord();
            this.eventDispatcher.notify('combinoscope.editor_enabled');
        },

        /**
         *
         */
        disableEditor: function () {
            this.editor = false;

            this.eventDispatcher.notify('combinoscope.editor_disabled');
        },

        /**
         * @returns {Layout}
         */
        getCurrentLayout: function () {
            return this.layouts.get(this.layout);
        },

        /**
         * @param {String} id
         */
        setCurrentLayout: function (id) {
            this.layout = id;

            this.clearWord();

            this.eventDispatcher.notify('combinoscope.layout_selected', {
                layout: this.getCurrentLayout(),
                id: id
            });
        },

        /**
         * @returns {String}
         */
        duplicateLayout: function () {
            var current = this.getCurrentLayout(),
                duplicate = Sankore.Layout.create(
                    current.name + ' (Copie)',
                    true
                ),
                id;

            duplicate.setLetters(current.letters.slice());

            id = Sankore.Util.UID.generate();

            this.layouts.set(id, duplicate);

            this.eventDispatcher.notify('combinoscope.layout_created', {
                layout: duplicate,
                id: id
            });

            return id;
        },

        /**
         * @param {String} id
         */
        deleteLayout: function () {
            if (this.getCurrentLayout().editable) {
                this.layouts.remove(this.layout);

                this.eventDispatcher.notify('combinoscope.layout_deleted', {
                    id: this.layout
                });
            }
        },

        /**
         * @param {String} name
         */
        renameLayout: function (name) {
            var layout = this.getCurrentLayout();
            if (layout.editable) {
                layout.name = name;

                this.eventDispatcher.notify('combinoscope.layout_renamed', {
                    name: name
                });
            }
        },

        /**
         * @returns {Boolean}
         */
        isThreeBlocksMode: function () {
            return this.threeBlocks;
        },

        /**
         * @param {Boolean} enabled
         */
        setThreeBlocksMode: function (enabled) {
            this.threeBlocks = enabled;

            this.clearWord();
            this.eventDispatcher.notify('combinoscope.mode_changed', {
                mode: this.threeBlocks
            });
        },

        /**
         * @param {String} letterId
         * @param {Boolean} before
         */
        addToWord: function (letterId, before) {
            before = (typeof before !== 'undefined' ? before : false);
            if (this.isLetterAllowed(letterId, before)) {
                this.word[before ? 'unshift' : 'push'](letterId);

                this.notifyWordChanged();
            }
        },

        /**
         * @param {String} letterId
         * @param {Boolean} before
         * @returns {Boolean}
         */
        isLetterAllowed: function (letterId, before) {
            var letter = this.letters.get(letterId);

            // provided letter id is invalid -> nope
            if (!letter) {
                return false;
            }

            // first letter -> ok
            if (this.word.length === 0) {
                return true;
            }

            // maximum word length exceeded -> nope
            if (this.word.length === (this.isThreeBlocksMode() ? 3 : 2)) {
                return false;
            }

            // provided letter and closest letter (depending on the position) have the same type -> nope
            if (letter.type === this.letters.get(this.word[before ? 0 : this.word.length - 1]).type) {
                return false;
            }

            return true;
        },

        /**
         * @param {Boolean} before
         */
        removeLetterFromWord: function (before) {
            if (this.word.length > 0) {
                this.word[(before || false) ? 'shift' : 'pop']();

                this.notifyWordChanged();
            }
        },

        /**
         *
         */
        clearWord: function () {
            this.word = [];

            this.notifyWordChanged();
        },

        /**
         *
         */
        notifyWordChanged: function () {
            var self = this;

            this.eventDispatcher.notify('combinoscope.word_changed', {
                word: this.word.map(function (id) {
                    return self.letters.get(id);
                })
            });
        },

        /**
         * @param {String} type optional
         * @returns {Array}
         */
        getCurrentLetters: function (type) {
            var layout = this.getCurrentLayout();
            if (!layout) {
                throw new Error('No layout is selected');
            }

            return this.letters.map(function (id, letter) {
                return layout.containLetter(id) && (typeof type === 'undefined' || type === letter.type) ? letter : null;
            });
        },

        /**
         * @param {String} letterId
         */
        selectLetter: function (letterId) {
            var layout = this.getCurrentLayout();

            if (layout.editable) {
                layout.addLetter(letterId);

                this.eventDispatcher.notify('combinoscope.letter_selected', {
                    letter: this.letters.get(letterId)
                });
            }
        },

        /**
         * @param {String} letterId
         */
        unselectLetter: function (letterId) {
            var layout = this.getCurrentLayout();

            if (layout.editable) {
                layout.removeLetter(letterId);

                this.eventDispatcher.notify('combinoscope.letter_unselected', {
                    letter: this.letters.get(letterId)
                });
            }
        },
        
        /**
         *
         */
        selectAllLetters: function () {
            var self = this;
            
            this.letters.map(function (id) {
                self.selectLetter(id);
            });
        },
        
        /**
         *
         */
        unselectAllLetters: function () {
            var self = this;
            
            this.letters.map(function (id) {
                self.unselectLetter(id);
            });
        }
    }));
})();