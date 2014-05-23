/* jshint browser:true */
/* globals klass:true, Sankore:true */
(function () {
    "use strict";

    klass.define('Sankore.UI', 'Interface', klass.extend({
        constructor: function (id, combinoscope) {
            this.id = id;
            this.combinoscope = combinoscope;
            this.dragdrop = Sankore.UI.DragDropManager.create(this.combinoscope.eventDispatcher);

            this.lettersZone = {};
            this.wordZone = null;
            this.editor = null;
            this.layoutSelectControl = null;
            this.layoutNameControl = null;
            this.layoutAddButton = null;
            this.layoutDeleteButton = null;
            this.layoutSelectAllButton = null;
            this.layoutUnselectAllButton = null;
            this.helpZone = null;

            this.attachEventHandlers();

            this._root = null;
        },

        /**
         *
         */
        attachEventHandlers: function () {
            var ed = this.combinoscope.eventDispatcher,
                self = this;

            ed.addEventListener('combinoscope.word_changed', function (event) {
                self.renderWord(event.word);
            });

            ed.addEventListener('combinoscope.layout_selected', function (event) {
                self.renderAllLetters();

                self.layoutSelectControl.value = event.id;

                if (self.combinoscope.editor) {
                    self.loadLayout(event.layout);
                }
            });

            ed.addEventSubscriber({
                events: ['combinoscope.layout_created', 'combinoscope.layout_deleted', 'combinoscope.layout_renamed'],
                listener: function () {
                    self.loadLayoutSelectControl();
                }
            });

            ed.addEventListener('combinoscope.editor_enabled', function () {
                self.showEditor();
            });

            ed.addEventListener('combinoscope.editor_disabled', function () {
                self.hideEditor();
            });

            ed.addEventListener('combinoscope.letter_selected', function (event) {
                self.selectLetter(event.letter);
            });

            ed.addEventListener('combinoscope.letter_unselected', function (event) {
                self.unselectLetter(event.letter);
            });

            ed.addEventListener('dragdrop_manager.dragstart', function (event) {
                if (event.group === 'add') {
                    self.showPlaceholders(event.source.dataset.id);
                }
            });

            ed.addEventListener('dragdrop_manager.dragend', function () {
                self.hidePlaceholders();
            });

            ed.addEventListener('dragdrop_manager.dragenter', function (event) {
                event.zone.classList.add('hover');
            });

            ed.addEventListener('dragdrop_manager.dragleave', function (event) {
                event.zone.classList.remove('hover');
            });

            ed.addEventListener('dragdrop_manager.drop', function (event) {
                switch (event.group) {
                case 'add':
                    ed.notify('ui.letter_dropped', {
                        letterId: event.source.dataset.id,
                        before: event.destination.dataset.position === 'left'
                    });
                    break;

                case 'remove':
                    ed.notify('ui.letter_removed', {
                        before: event.source.dataset.index === '0'
                    });
                    break;
                }
            });
        },

        /**
         *
         */
        _clearElement: function (element) {
            while (element.firstChild) {
                element.removeChild(element.firstChild);
            }
        },

        /**
         *
         */
        render: function () {
            this.createBaseMarkup();

            this.loadLayoutSelectControl();

            this.renderAllLetters();
            this.renderWord([]);
        },

        /**
         *
         */
        renderAllLetters: function () {
            this.renderLetters('C');
            this.renderLetters('V');
        },

        /**
         * @param {String} type
         */
        renderLetters: function (type) {
            var zone = this.lettersZone[type],
                letters,
                i;

            if (!zone) {
                throw new Error('"' + type + '" zone is not rendered yet');
            }

            var oldLetters = zone.getElementsByClassName('letter');
            while (oldLetters.length > 0) {
                zone.removeChild(oldLetters[0]);
            }

            if (!this.combinoscope.editor) {
                letters = this.combinoscope.getCurrentLetters(type);
            } else {
                letters = this.combinoscope.letters.map(function (id, letter) {
                    return letter.type === type ? letter : null;
                });
            }

            if (letters) {
                var fragment = document.createDocumentFragment(),
                    letter;

                for (i in letters) {
                    if (!this.combinoscope.editor) {
                        letter = this.createLetter(letters[i], true);
                    } else {
                        letter = this.createEditorLetter(letters[i]);
                    }
                    fragment.appendChild(letter);
                }

                zone.appendChild(fragment);
            }
        },

        /**
         * @param {Array} word
         */
        renderWord: function (word) {
            var self = this,
                container = this.wordZone.querySelector('.container'),
                placeholders = this.wordZone.getElementsByClassName('placeholder'),
                length = word.length,
                letter,
                i = 0;

            while (placeholders.length > 0) {
                this.wordZone.removeChild(placeholders[0]);
            }

            this._clearElement(container);

            this.wordZone.insertBefore(this.createPlaceholder('left'), container);

            word.forEach(function (letter, index) {
                letter = self.createLetter(
                    letter,
                    index === 0 || index === word.length - 1,
                    index
                );

                if (0 === i) {
                    letter.classList.add('first');
                }

                if (length - 1 === i) {
                    letter.classList.add('last');
                }

                container.appendChild(letter);
                i++;
            });

            if (word.length > 0) {
                this.wordZone.appendChild(this.createPlaceholder('right'));
            }
        },

        /**
         *
         * @param {Letter} letter
         * @returns {HTMLElement}
         */
        createLetter: function (letter, draggable, index) {
            var element = this.createBaseLetterSpan(),
                group = 'add';

            switch (letter.type) {
            case 'C':
                element.classList.add('consonant');
                break;

            case 'V':
                element.classList.add('vowel');
                break;
            }

            element.dataset.id = letter.id;
            element.appendChild(document.createTextNode(letter.name));

            if (typeof index !== 'undefined') {
                element.dataset.index = index;
                group = 'remove';
            }

            if (draggable) {
                this.dragdrop.registerDraggable(element, group);
            }

            return element;
        },

        /**
         *
         */
        createEditorLetter: function (letter) {
            var element = this.createLetter(letter, false),
                ed = this.combinoscope.eventDispatcher;

            element.addEventListener('click', function (e) {
                ed.notify('ui.letter_clicked', {
                    letterId: letter.id,
                    active: !element.classList.contains('disabled')
                });
            });

            return element;
        },

        /**
         * @returns {HTMLElement}
         */
        createPlaceholder: function (position) {
            var element = this.createBaseLetterSpan();

            element.classList.add('placeholder');
            element.dataset.position = typeof position === 'undefined' ? 'left' : position;
            element.style.visibility = 'hidden';

            this.dragdrop.registerDropzone(element, 'add');

            return element;
        },

        /**
         * @returns {HTMLElement}
         */
        createBaseLetterSpan: function () {
            var element = document.createElement('span');

            element.className = 'letter';

            return element;
        },

        /**
         * @returns {HTMLElement}
         */
        getRootElement: function () {
            if (this._root === null) {
                this._root = document.getElementById(this.id);
            }

            return this._root;
        },

        /**
         *
         */
        createBaseMarkup: function () {
            var root = this.getRootElement(),
                fragment = document.createDocumentFragment(),
                title,
                items = [];

            this._clearElement(root);

            root.classList.add('combinoscope');

            // header
            items[0] = document.createElement('div');
            items[0].className = 'header';

            title = document.createElement('h1');
            title.appendChild(document.createTextNode('Combinoscope'));
            items[0].appendChild(title);
            items[0].appendChild(this.createControls());

            // letters zone
            items[1] = document.createElement('div');
            items[1].className = 'letters';
            items[1].appendChild(this.createLettersZoneMarkup('C'));
            items[1].appendChild(this.createLettersZoneMarkup('V'));

            items.push(this.wordZone = this.createWordZone());

            items.push(this.editor = this.createEditor());

            for (var i in items) {
                fragment.appendChild(items[i]);
            }

            root.appendChild(fragment);
        },

        /**
         * @returns {HTMLElement}
         */
        createWordZone: function () {
            var clearButton = document.createElement('a'),
                ed = this.combinoscope.eventDispatcher,
                zone = document.createElement('div'),
                container = document.createElement('div');

            zone.className = 'word';
            container.className = 'container';

            clearButton.appendChild(document.createTextNode('Recommencer'));
            clearButton.className = 'clear';
            clearButton.addEventListener('click', function (e) {
                e.preventDefault();
                ed.notify('ui.clear_clicked');
            });

            zone.appendChild(clearButton);
            zone.appendChild(container);

            return zone;
        },

        /**
         * @returns {HTMLElement}
         */
        createEditor: function () {
            var editor = document.createElement('div');
            editor.className = 'editor';

            editor.style.display = 'none';

            editor.appendChild(this.wrapControl(
                this.layoutAddButton = this.createButton('Ajouter', 'ui.add_layout_clicked'),
                'add_layout_control'
            ));

            editor.appendChild(this.wrapControl(
                this.layoutNameControl = this.createLayoutNameControl(),
                'layout_name_control',
                'Nom du groupe'
            ));

            editor.appendChild(this.wrapControl(
                this.layoutSelectAllButton = this.createButton('Tout sélectionner', 'ui.select_all_layout_clicked'),
                'select_all_layout_control'
            ));

            editor.appendChild(this.wrapControl(
                this.layoutUnselectAllButton = this.createButton('Tout désélectionner', 'ui.unselect_all_layout_clicked'),
                'unselect_all_layout_control'
            ));

            editor.appendChild(this.wrapControl(
                this.layoutDeleteButton = this.createButton('Supprimer', 'ui.delete_layout_clicked'),
                'delete_layout_control'
            ));

            this.helpZone = document.createElement('div');
            this.helpZone.className = 'help';

            editor.appendChild(this.helpZone);

            return editor;
        },

        /**
         *
         */
        showEditor: function () {
            this.editor.style.display = 'block';
            this.wordZone.style.display = 'none';

            this.renderAllLetters();

            this.loadLayout(this.combinoscope.getCurrentLayout());
        },

        /**
         *
         */
        hideEditor: function () {
            this.editor.style.display = 'none';
            this.wordZone.style.display = 'block';

            this.renderAllLetters();
        },

        /**
         * @param {Layout} layout
         */
        loadLayout: function (layout) {
            var self = this;

            this.layoutNameControl.value = layout.name;
            this.layoutNameControl.disabled = !layout.editable;

            this.layoutDeleteButton.disabled = !layout.editable;
            this.layoutSelectAllButton.disabled = !layout.editable;
            this.layoutUnselectAllButton.disabled = !layout.editable;

            this.combinoscope.letters.map(function (id, letter) {
                if (layout.containLetter(id)) {
                    self.selectLetter(letter);
                } else {
                    self.unselectLetter(letter);
                }
            });

            this.updateHelpZone(layout);
        },

        /**
         * @param {String} type
         * @returns {HTMLElement}
         */
        createLettersZoneMarkup: function (type) {
            this.lettersZone[type] = document.createElement('div');
            var title = document.createElement('h2'),
                name, className;

            switch (type) {
            case 'V':
                name = 'Voyelles';
                className = 'vowels';
                break;

            case 'C':
                name = 'Consonnes';
                className = 'consonants';
                break;
            }

            this.lettersZone[type].className = className;

            title.appendChild(document.createTextNode(name));
            this.lettersZone[type].appendChild(title);

            this.dragdrop.registerDropzone(this.lettersZone[type], 'remove');

            return this.lettersZone[type];
        },

        /**
         * @returns {HTMLElement}
         */
        createControls: function () {
            var controls = document.createDocumentFragment();

            controls.appendChild(this.wrapControl(
                this.createModeControl(),
                'mode_control',
                'Blocs'
            ));

            controls.appendChild(this.wrapControl(
                this.layoutSelectControl = this.createLayoutSelectControl(),
                'layout_control',
                'Groupe'
            ));

            controls.appendChild(this.wrapControl(
                this.createButton('Editeur', 'ui.editor_clicked'),
                'editor_control'
            ));

            return controls;
        },

        /**
         * @returns {HTMLElement}
         */
        createModeControl: function () {
            var items = document.createDocumentFragment(),
                span2 = document.createElement('span'),
                span3 = document.createElement('span'),
                input = document.createElement('input'),
                ed = this.combinoscope.eventDispatcher;

            span2.appendChild(document.createTextNode('2'));
            span3.appendChild(document.createTextNode('3'));

            input.setAttribute('type', 'checkbox');
            input.setAttribute('id', 'mode_control');

            input.addEventListener('change', function (event) {
                event.preventDefault();

                ed.notify('ui.mode_changed', {
                    mode: this.checked
                });
            });

            input.checked = this.combinoscope.isThreeBlocksMode();

            items.appendChild(span2);
            items.appendChild(input);
            items.appendChild(span3);

            return items;
        },

        /**
         * @returns {HTMLElement}
         */
        createLayoutSelectControl: function () {
            var select = document.createElement('select'),
                ed = this.combinoscope.eventDispatcher;

            select.setAttribute('id', 'layout_control');

            select.addEventListener('change', function (e) {
                ed.notify('ui.layout_selected', {
                    layout: select.value
                });
            });

            return select;
        },

        /**
         *
         */
        createLayoutNameControl: function () {
            var input = document.createElement('input'),
                ed = this.combinoscope.eventDispatcher;

            input.type = 'text';
            input.id = 'name_control';
            input.required = true;
            input.pattern = '.{1,30}';

            input.addEventListener('keyup', function (e) {
                ed.notify('ui.layout_name_changed', {
                    name: this.value
                });
            });

            return input;
        },

        /**
         * @returns {HTMLElement}
         */
        createButton: function (name, eventName) {
            var button = document.createElement('button'),
                ed = this.combinoscope.eventDispatcher;

            button.appendChild(document.createTextNode(name));
            button.type = 'button';

            button.addEventListener('click', function (e) {
                e.preventDefault();

                ed.notify(eventName);
            });

            return button;
        },

        /**
         *
         */
        loadLayoutSelectControl: function () {
            var option, select = this.layoutSelectControl;

            this._clearElement(select);

            this.combinoscope.layouts.map(function (id, layout) {
                option = document.createElement('option');
                option.text = layout.name;
                option.value = id;
                select.add(option);
            });

            select.value = this.combinoscope.layout;
        },

        /**
         * @param {String} id
         * @param {HTMLElement} element
         * @returns {HTMLElement}
         */
        wrapControl: function (element, id, name) {
            var div = document.createElement('div');

            div.className = 'control';

            if (typeof id !== 'undefined') {
                div.classList.add(id.replace(/_/g, '-'));

                if (typeof name !== 'undefined') {
                    var label = document.createElement('label');

                    label.setAttribute('for', id);
                    label.appendChild(document.createTextNode(name));

                    div.appendChild(label);
                }
            }

            div.appendChild(element);

            return div;
        },

        /**
         * @param {String} letterId
         */
        showPlaceholders: function (letterId) {
            var self = this;

            Array.prototype.forEach.call(
                this.getAllPlaceholders(),
                function (placeholder) {
                    if (self.combinoscope.isLetterAllowed(letterId, placeholder.dataset.position === 'left')) {
                        placeholder.style.visibility = 'visible';
                    }
                }
            );
        },

        /**
         *
         */
        hidePlaceholders: function ()  {
            Array.prototype.forEach.call(
                this.getAllPlaceholders(),
                function (placeholder) {
                    placeholder.style.visibility = 'hidden';
                }
            );
        },

        /**
         * @returns {NodeList}
         */
        getAllPlaceholders: function () {
            return this.wordZone.querySelectorAll('.placeholder');
        },

        /**
         * @param {String} letterId
         */
        selectLetter: function (letter) {
            var zone = this.lettersZone[letter.type],
                element;

            if (zone) {
                element = zone.querySelector('[data-id="' + letter.id + '"]');

                if (element) {
                    element.classList.remove('disabled');
                }
            }
        },

        /**
         * @param {String} letterId
         */
        unselectLetter: function (letter) {
            var zone = this.lettersZone[letter.type],
                element;

            if (zone) {
                element = zone.querySelector('[data-id="' + letter.id + '"]');

                if (element) {
                    element.classList.add('disabled');
                }
            }
        },

        /**
         * @param {Layout} layout
         */
        updateHelpZone: function (layout) {
            if (layout.editable) {
                this.helpZone.innerText = 'Sélectionner les lettres que vous souhaitez faire apparaître en cliquant dessus';
            } else {
                this.helpZone.innerText = 'Ce groupe n\'est pas modifiable. Cliquez sur le bouton + pour créer un nouveau groupe';
            }
        },

        /**
         * @returns {Number}
         */
        getTotalHeight: function () {
            var height = 0,
                children = this.getRootElement().childNodes;

            for (var i = 0; i < children.length; i++) {
                height += children[i].offsetHeight;
            }

            return height;
        }
    }));
})();