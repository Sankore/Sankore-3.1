/*jslint browser: true, devel: true, plusplus: true, nomen: true*/
/*global tinymce: true, jQuery: true*/
(function ($) {
    "use strict";
    var app = {};

    app.RTEditor = function (id, widget, options) {

        var allowedDroppedContentTyped = [
            'image/png', 'image/jpg', 'image/jpeg', 'image/gif', 'image/svg+xml', 'image/tiff'
        ];

        this.id = id;
        this.empty = true;
        this.font = {
            name: 'arial',
            size: '12pt'
        };
        this.widget = widget;
        this.dark = false;
        this.backgroundColor = 'transparent';
        this.options = {};
        this.$container = null;

        if (app.RTEditor.defined === undefined) {

            /**
             * Init the widget
             */
            app.RTEditor.prototype.init = function (id, options) {
                this.$container = $('#' + id);
                this.$container.addClass('rteditor');

                this.initOptions(options);
                this.initTinymce();

                this.attachEventHandlers();
            };

            /**
             * Init and process options
             */
            app.RTEditor.prototype.initOptions = function (options) {
                this.options = $.extend({}, {
                    defaultText: this.$container.html(),
                    locale: 'en_GB',
                    autoShow: false,
                    dark: false,
                    onLinkClick: function (a) {},
                    onInit: function () {},
                    onBlur: function () {},
                    onFocus: function () {},
                    onContentOverflow: function (delta) {},
                    onFontSizeChange: function (fontSize) {},
                    onFontFamilyChange: function (fontFamily) {},
                    onFontFormatChange: function (name) {},
                    onWidgetFocus: function () {},
                    onWidgetBlur: function () {}
                }, options);
            };

            /**
             * Init the TinyMCE editor instance
             */
            app.RTEditor.prototype.initTinymce = function () {
                var self = this,
                    options = {
                        language: this.options.locale,
                        auto_focus: id,
                        statusbar: false,
                        menubar: false,
                        skin: 'rteditor',
                        target_list: false,
                        relative_urls: true,
                        convert_urls: false,
                        forced_root_block: 'p',
                        force_p_newlines: true,
                        valid_elements : '*[*]',
                        keep_styles: true,
                        plugins: ['link', 'searchreplace', 'table', 'paste', 'textcolor', 'rteditor'],
                        toolbar1: 'bold italic underline strikethrough | forecolor backcolor pagecolor | link | undo redo',
                        toolbar2: 'fontselect fontsizeselect | alignleft aligncenter alignright alignjustify | customtable'
                    };

                options.fontsize_formats = [
                    '6pt', '7pt', '8pt', '9pt', '10pt', '11pt', '12pt',
                    '14pt', '16pt', '18pt', '20pt', '22pt', '24pt', '26pt', '28pt',
                    '36pt', '48pt', '72pt'
                ].join(' ');

                options.font_formats = [
                    'Alphonetic=alphonetic',
                    'Alphonetic GB=alphoneticGB',
                    'Andika Basic=andika basic',
                    'Arial=arial',
                    'Arial Black=arial black',
                    'Comic Sans MS=comic sans ms',
                    'Courier New=courier new',
                    'Cursive Standard=cursive standard',
                    'Écolier=ecolier',
                    'Écolier (court)=ecolier_court',
                    'Écolier CP=ecolier_cp',
                    'Écolier CP (pointillés)=ecolier_cp_pointillés',
                    'Écolier lignes=ecolier_lignes',
                    'Écolier lignes (court)=ecolier_lignes_court',
                    'Écolier lignes (pointillés)=ecolier_lignes_pointillés',
                    'Écolier (pointillés)=ecolier_pointillés',
                    'Écriture A=ecriture a',
                    'Écriture B=ecriture b',
                    'Georgia=georgia',
                    'Gino School Script=ginoschoolscript',
                    'Impact=impact',
                    'Script École=script cole',
                    'Script École 2=script ecole 2',
                    'Scriptcase cole=scriptcase cole',
                    'Times New Roman=times new roman,times',
                    'Trebuchet MS=trebuchet ms',
                    'Verdana=verdana'
                ].join(';');

                options.setup = function (editor) {
                    editor.on('click', function (e) {
                        self.onTinyClick(e);
                    });

                    editor.on('init', function (e) {
                        self.onTinyInit(e);
                    });

                    editor.on('blur', function (e) {
                        self.onTinyBlur(e);
                    });

                    editor.on('focus', function (e) {
                        self.onTinyFocus(e);
                    });

                    editor.on('show', function (e) {
                        self.onTinyShow(e);
                    });

                    editor.on('ExecCommand', function (e) {
                        self.onTinyCommand(e);
                    });
                    
                    editor.on('PreProcess', function (e) {
                        $(e.node).find('[data-mce-bogus]').replaceWith('<br>');
                    });
                };

                tinymce.PluginManager.add('rteditor', function (editor, url) {
                    var onPostRender = function () {
                        var self = this;

                        function bindStateListener() {
                            self.disabled(!editor.dom.getParent(editor.selection.getStart(), 'td,th'));

                            editor.selection.selectorChanged('td,th', function (state) {
                                self.disabled(!state);
                            });
                        }

                        if (editor.initialized) {
                            bindStateListener();
                        } else {
                            editor.on('init', bindStateListener);
                        }
                    };

                    editor.addMenuItem('row', {
                        text: 'Row',
                        context: 'table',
                        menu: [
                            {
                                text: 'Insert row before',
                                onclick: function () {
                                    editor.execCommand('mceTableInsertRowBefore');
                                },
                                onPostRender: onPostRender
                            },
                            {
                                text: 'Insert row after',
                                onclick: function () {
                                    editor.execCommand('mceTableInsertRowAfter');
                                },
                                onPostRender: onPostRender
                            },
                            {
                                text: 'Delete row',
                                onclick: function () {
                                    editor.execCommand('mceTableDeleteRow');
                                },
                                onPostRender: onPostRender
                            }
                        ]
                    });

                    editor.addButton('customtable', {
                        type: 'menubutton',
                        text: false,
                        icon: 'table',
                        menu: [
                            editor.menuItems.inserttable,
                            editor.menuItems.column,
                            editor.menuItems.row,
                            {
                                text: 'Merge cells',
                                onclick: function () {
                                    var cell = editor.dom.getParent(editor.selection.getStart(), 'th,td');

                                    editor.execCommand('mceTableMergeCells');

                                    if (cell) {
                                        cell.innerHTML = cell.innerHTML.replace(/(<br>\s*)+/, '<br>');
                                    }
                                },
                                onPostRender: onPostRender
                            },
                            {
                                text: 'Split cell',
                                onclick: function () {
                                    editor.execCommand('mceTableSplitCells');
                                },
                                onPostRender: onPostRender
                            },
                            {
                                text: 'Cell properties', 
                                onclick: function () {
                                    editor.execCommand('mceTableCellProps');
                                }, 
                                onPostRender: onPostRender
                            }
                        ]
                    });

                    function mapColors() {
                        var i,
                            colors = [],
                            colorMap;

                        colorMap = editor.settings.textcolor_map || [
                            "000000", "Black",
                            "993300", "Burnt orange",
                            "333300", "Dark olive",
                            "003300", "Dark green",
                            "003366", "Dark azure",
                            "000080", "Navy Blue",
                            "333399", "Indigo",
                            "333333", "Very dark gray",
                            "800000", "Maroon",
                            "FF6600", "Orange",
                            "808000", "Olive",
                            "008000", "Green",
                            "008080", "Teal",
                            "0000FF", "Blue",
                            "666699", "Grayish blue",
                            "808080", "Gray",
                            "FF0000", "Red",
                            "FF9900", "Amber",
                            "99CC00", "Yellow green",
                            "339966", "Sea green",
                            "33CCCC", "Turquoise",
                            "3366FF", "Royal blue",
                            "800080", "Purple",
                            "999999", "Medium gray",
                            "FF00FF", "Magenta",
                            "FFCC00", "Gold",
                            "FFFF00", "Yellow",
                            "00FF00", "Lime",
                            "00FFFF", "Aqua",
                            "00CCFF", "Sky blue",
                            "993366", "Brown",
                            "C0C0C0", "Silver",
                            "FF99CC", "Pink",
                            "FFCC99", "Peach",
                            "FFFF99", "Light yellow",
                            "CCFFCC", "Pale green",
                            "CCFFFF", "Pale cyan",
                            "99CCFF", "Light sky blue",
                            "CC99FF", "Plum",
                            "FFFFFF", "White",
                            "transparent", "Transparent"
                        ];

                        for (i = 0; i < colorMap.length; i += 2) {
                            colors.push({
                                text: colorMap[i + 1],
                                color: colorMap[i]
                            });
                        }

                        return colors;
                    }

                    editor.addButton('pagecolor', {
                        type: 'colorbutton',
                        tooltip: 'Page color',
                        icon: 'page',
                        panel: {
                            html: function () {
                                var ctrl = this,
                                    colors,
                                    color,
                                    html,
                                    last,
                                    rows,
                                    cols,
                                    x,
                                    y,
                                    i;

                                colors = mapColors();

                                html = '<table class="mce-grid mce-grid-border mce-colorbutton-grid" role="presentation" cellspacing="0"><tbody>';
                                last = colors.length - 1;
                                rows = editor.settings.textcolor_rows || 5;
                                cols = editor.settings.textcolor_cols || 8;

                                for (y = 0; y < rows; y++) {
                                    html += '<tr>';

                                    for (x = 0; x < cols; x++) {
                                        i = y * cols + x;

                                        if (i > last) {
                                            html += '<td></td>';
                                        } else {
                                            color = colors[i];
                                            html += (
                                                '<td>' +
                                                '<div id="' + ctrl._id + '-' + i + '"' +
                                                ' data-mce-color="' + color.color + '"' +
                                                ' role="option"' +
                                                ' tabIndex="-1"' +
                                                ' style="' + (color ? 'background-color: #' + color.color : '') + '"' +
                                                ' title="' + color.text + '">' +
                                                '</div>' +
                                                '</td>'
                                            );
                                        }
                                    }

                                    html += '</tr>';
                                }

                                html += '</tbody></table>';

                                return html;
                            },
                            onclick: function (e) {
                                var buttonCtrl = this.parent(),
                                    value = e.target.getAttribute('data-mce-color');

                                if (value) {
                                    buttonCtrl.hidePanel();
                                    value = '#' + value;
                                    buttonCtrl.color(value);
                                    self.setBackgroundColor(value);
                                }
                            }
                        },
                        onclick: function () {
                            if (this._color) {
                                self.setBackgroundColor(this._color);
                            }
                        }
                    });
                });

                this.tinymce = new tinymce.Editor(id, options, tinymce.EditorManager);
                this.tinymce.render();
            };

            /**
             * Attach all the event handlers
             */
            app.RTEditor.prototype.attachEventHandlers = function () {
                var self = this;

                $(window).bind('resize', function () {
                    self.refreshSize();
                });

                $('#' + this.id).on('click', 'a', function (e) {
                    self.options.onLinkClick.call(self, this);
                });

                if (this.widget) {
                    this.widget.onfocus = function () {
                        if (window && window.sankore && window.sankore.currentToolIsSelector()) {
                            self.show();

                            if (self.empty) {
                                self.setContent('');
                            }

                            self.options.onWidgetFocus.call(self);
                        }
                    };

                    this.widget.onblur = function () {
                        self.tinymce.fire('blur');

                        var content = self.getContent();
                        
                        if (self.empty = app.RTEditor.isContentEmpty(content)) {
                            self.setContent(tinymce.translate(self.options.defaultText));
                        }

                        self.hide();

                        self.options.onWidgetBlur.call(self);
                    };

                    this.widget.ondrop = function (url, contentType) {
                        if (self.isAllowedDroppedContentType(contentType)) {
                            self.addDroppedContent(url);
                        }
                    };

                    this.widget.onbackgroundswitch = function (dark) {
                        self.setDarkBackground(dark);
                    };
                }
            };

            /**
             * Gets the current iframe element. May be null if invoked before the tinyMCE editor is initialized
             */
            app.RTEditor.prototype.getIframe = function () {
                return $('#' + this.id + '_ifr');
            };

            /**
             * Recompute the editor instance size to fit into the viewport
             */
            app.RTEditor.prototype.refreshSize = function () {
                if (this.tinymce.theme) {
                    var $win = $(window);

                    this.tinymce.theme.resizeTo(
                        $win.width(),
                        $win.height()
                    );
                }
            };

            /**
             * Show the editor
             */
            app.RTEditor.prototype.show = function () {
                this.tinymce.show();
                $('body').removeClass('view');
            };

            /**
             * Hide the editor
             */
            app.RTEditor.prototype.hide = function () {
                this.tinymce.hide();
                $('body').addClass('view');
            };

            /**
             * Change the background independently to the dark background feature
             */
            app.RTEditor.prototype.setBackgroundColor = function (color) {
                if ((color === '#FFFFFF' && !this.dark) || (color === '#000000' && this.dark)) {
                    color = 'transparent';
                }

                this.backgroundColor = color;

                $(this.tinymce.getDoc().body).css('background-color', color);
                this.$container.css('background-color', color);
            };

            /**
             * Switch background to inverted colors (preserving color other than white and black)
             */
            app.RTEditor.prototype.setDarkBackground = function (dark) {
                dark = !!dark;

                var $docBody = $(this.tinymce.getDoc().body),
                    darkClassname = 'dark',
                    cleanColorSpan = function ($body, color) {
                        $body.find('span[style],td[style]').each(function () {
                            var $el = $(this);

                            if ($el.is('span') && $el.css('color') === color) {
                                $el.css('color', 'inherit');
                            }

                            if ($el.is('td') && $el.css('background-color') === color) {
                                $el.css('background-color', 'inherit');
                            }
                        });
                    };

                if (dark) {
                    $docBody.addClass(darkClassname);
                    this.$container.addClass(darkClassname);

                    if (this.backgroundColor !== 'transparent') {
                        cleanColorSpan($docBody, 'rgb(0, 0, 0)');
                    }
                } else {
                    $docBody.removeClass(darkClassname);
                    this.$container.removeClass(darkClassname);

                    if (this.backgroundColor !== 'transparent') {
                        cleanColorSpan($docBody, 'rgb(255, 255, 255)');
                    }
                }

                this.dark = dark;
            };

            /**
             *
             */
            app.RTEditor.prototype.getContent = function () {
                return this.tinymce.getContent();
            };

            /**
             *
             */
            app.RTEditor.prototype.setContent = function (content) {
                this.tinymce.setContent(content);
                this.tinymce.save();
            };

            /**
             *
             */
            app.RTEditor.prototype.setDefaultFontFamily = function (name) {
                if (name.length > 0) {
                    this.tinymce.execCommand('FontName', true, name);
                    this.font.name = name;
                }
            };

            /**
             *
             */
            app.RTEditor.prototype.setDefaultFontBold = function (isBold) {
                if (isBold) {
                    this.tinymce.execCommand('Bold', true);
                }
            };

            /**
             *
             */
            app.RTEditor.prototype.setDefaultFontItalic = function (isItalic) {
                if (isItalic) {
                    this.tinymce.execCommand('Italic', true);
                }
            };

            /**
             *
             */
            app.RTEditor.prototype.setDefaultFontSize = function (size) {
                size = size.replace('pt', '');
                if (!isNaN(Number(size)) && Number(size) > 0) {
                    this.tinymce.execCommand('FontSize', true, size + 'pt');
                    this.font.size = size + 'pt';
                }
            };

            /**
             * Validate dropped content type. Image only
             */
            app.RTEditor.prototype.isAllowedDroppedContentType = function (contentType) {
                return $.inArray(contentType, allowedDroppedContentTyped) !== -1;
            };

            /**
             * Add dropped image url
             */
            app.RTEditor.prototype.addDroppedContent = function (url) {
                var img = new Image();
                img.src = url;
                img.alt = url;

                this.tinymce.selection.getRng().insertNode(img);
            };

            /**
             *
             */
            app.RTEditor.prototype.checkForResize = function () {
                var delta = $(this.tinymce.getDoc()).height() - this.getIframe().height();

                if (delta > 0) {
                    this.options.onContentOverflow.call(this, delta);
                }
            };

            /**
             * Event handler for TinyMCE editor 'init' event
             */
            app.RTEditor.prototype.onTinyInit = function (e) {
                var self = this,
                    editor = this.tinymce;

                this.options.onInit.call(this);

                this.tinymce.editorCommands.addCommands({
                    'HiliteColor': function (command, ui, value) {
                        var set = false;
                        if (editor.selection.selectedRange && editor.selection.selectedRange.commonAncestorContainer) {
                            var $container = $(editor.selection.selectedRange.commonAncestorContainer),
                                tds = $container.parents('table').find('.mce-item-selected').toArray();

                            if ($container.is('td')) {
                                tds.push($container.get(0));
                            }

                            if ($container.closest('td').length) {
                                tds.push($container.closest('td').get(0));
                            }

                            if (tds.length > 0) {
                                $(tds).css('background-color', value);
                                set = true;
                            }
                        }

                        if (!set) {
                            editor.formatter.toggle(command, value ? {
                                value: value
                            } : undefined);
                        }

                        editor.nodeChanged();
                    }
                });

                this.getIframe().attr('scrolling', 'no');

                var handler = function (e) {
                    self.checkForResize();
                };

                $(editor.getDoc()).bind('keydown', handler);

                // hack dégueulasse pour se passer de confirmation dans le cas d'ajout de lien externe
                var confirm = this.tinymce.windowManager.confirm;
                this.tinymce.windowManager.confirm = function (message, callback, scope) {
                    if (message.indexOf('external link') !== -1) {
                        callback.call(scope || this, true);
                    } else {
                        confirm.call(this, message, callback, scope);
                    }
                };

                this.checkForResize();

                if (!this.options.autoShow) {
                    this.hide();
                }

                this.setDarkBackground(this.options.dark);
            };

            /**
             * Event handler for TinyMCE editor 'blur' event
             */
            app.RTEditor.prototype.onTinyBlur = function (e) {
                this.options.onBlur.call(this);
            };

            /**
             * Event handler for TinyMCE editor 'blur' event
             */
            app.RTEditor.prototype.onTinyFocus = function (e) {
                this.options.onFocus.call(this);
            };

            /**
             * Event handler for TinyMCE editor 'show' event
             */
            app.RTEditor.prototype.onTinyShow = function (e) {
                this.tinymce.fire('focus');
                this.refreshSize();
            };

            /**
             * Event handler for TinyMCE editor 'click' event
             */
            app.RTEditor.prototype.onTinyClick = function (e) {
                var clicked = $(e.target);

                if (clicked.closest('a').length) {
                    e.preventDefault();
                    this.options.onLinkClick.call(this, clicked.closest('a').get(0));

                    return false;
                }
            };

            /**
             * Event handler for TinyMCE editor 'ExecCommand' event
             */
            app.RTEditor.prototype.onTinyCommand = function (e) {
                if (e.command === 'mceToggleFormat') {
                    this.options.onFontFormatChange.call(this, e.value);
                }

                if (e.command === 'FontName') {
                    this.options.onFontFamilyChange.call(this, e.value);
                    this.font.name = e.value;
                }

                if (e.command === 'FontSize') {
                    this.options.onFontSizeChange.call(this, e.value);
                    this.font.size = e.value;
                }

                if (e.command === 'mceInsertContent' && e.value.indexOf('<table>') !== -1) {
                    this.onTinyTableCreate($(this.tinymce.selection.getNode()).parents('table'));
                }

                this.checkForResize();
            };

            /**
             * Custom event handler for table creation
             */
            app.RTEditor.prototype.onTinyTableCreate = function ($table) {
                $table.find('td,th').css({
                    fontFamily: this.font.name,
                    fontSize: this.font.size
                });
            };

            app.RTEditor.defined = true;
        }

        try {
            this.init(id, options);
        } catch (e) {
            console.error(e.message);
            console.error(e.stack);
        }
    };

    app.RTEditor.isContentEmpty = function (content) {
        return $('<div>' + content + '</div>').text().trim().length === 0;
    };

    $(document).ready(function () {
        var options = {
            locale: 'en_GB',
            defaultText: 'Insert your text here',
            autoShow: false
        }, widget = null;

        if (window.sankore) {
            options.onLinkClick = function (a) {
                window.sankore.loadUrl($(a).attr('href'));
            };

            options.onInit = function () {
                var text = window.sankore.preference('content', '');

                this.setContent(text);

                if (!app.RTEditor.isContentEmpty(text)) {
                    this.empty = false;
                }

                if ('true' === window.sankore.preference('dark', 'false')) {
                    this.setDarkBackground(true);
                }

                this.setBackgroundColor(window.sankore.preference('background'));
            };

            options.onBlur = function () {
                window.sankore.setPreference('content', this.getContent());
                window.sankore.setPreference('dark', this.dark ? 'true' : 'false');
                window.sankore.setPreference('background', this.backgroundColor);
            };

            options.onFocus = function () {
                if (this.empty) {
                    this.setDefaultFontFamily(window.sankore.fontFamilyPreference());
                    this.setDefaultFontSize(window.sankore.fontSizePreference());
                    this.setDefaultFontBold(window.sankore.fontBoldPreference());
                    this.setDefaultFontItalic(window.sankore.fontItalicPreference());
                }
            };

            options.onContentOverflow = function (delta) {
                var $win = $(window);

                window.sankore.resize(
                    $win.width(),
                    $win.height() + delta
                );
            };

            options.onFontFamilyChange = function (name) {
                window.sankore.updateFontFamilyPreference(name);
            };

            options.onFontSizeChange = function (size) {
                window.sankore.updateFontSizePreference(size.replace('pt', ''));
            };

            options.onFontFormatChange = function (name) {
                if (name === 'bold') {
                    window.sankore.updateFontBoldPreference();
                } else if (name === 'italic') {
                    window.sankore.updateFontItalicPreference();
                }
            };

//            options.onWidgetFocus = function () {
//                if (this.empty) {
//                    this.setDefaultFontFamily(window.sankore.fontFamilyPreference());
//                    this.setDefaultFontSize(window.sankore.fontSizePreference());
//                    this.setDefaultFontBold(window.sankore.fontBoldPreference());
//                    this.setDefaultFontItalic(window.sankore.fontItalicPreference());
//                }
//            };

            options.locale = window.sankore.locale();
            options.dark = window.sankore.isDarkBackground();
        }

        if (window.widget) {
            widget = window.widget;

            window.widget.onChangeBackground = function () {
                if (window.sankore && window.widget.onbackgroundswitch) {
                    window.widget.onbackgroundswitch(window.sankore.isDarkBackground());
                }
            };
        }

        if (window.sankore) {
            if (window.sankore.preference('loaded').length === 0) {
                options.autoShow = true;
                window.sankore.setPreference('loaded', 'true');
            }
        }

        window.rteditor = new app.RTEditor('ubwidget', widget, options);
    });
}(jQuery));

/** mockup widget object for browser testing */
if (!('widget' in window)) {
    window.widget = {};
}

/** mockup sankore object for browser testing */
if (!('sankore' in window)) {
    var preferences = {};

    window.sankore = {
        loadUrl: function (url) {
            window.open(url);
        },

        isDarkBackground: function () {
            return false;
        },

        setPreference: function (name, value) {
            preferences[name] = value;
        },

        updateFontFamilyPreference: function (name) {
            console.log('Default font family set to : ' + name);
        },

        fontFamilyPreference: function () {
            return 'georgia';
        },

        fontSizePreference: function () {
            return '16pt';
        },

        updateFontSizePreference: function (name) {
            console.log('Default font size set to : ' + name);
        },

        updateFontBoldPreference: function (bold) {
            console.log('Default bold set to :', bold);
        },

        fontBoldPreference: function () {
            return false;
        },

        updateFontItalicPreference: function (italic) {
            console.log('Default italic set to :', italic);
        },

        fontItalicPreference: function () {
            return false;
        },

        resize: function (w, h) {
            console.log('Resizing window to [' + w + ', ' + h + ']');
        },

        preference: function (name, def) {
            if (name in preferences) {
                return preferences[name];
            }

            if (typeof def !== 'undefined') {
                return def;
            }

            return '';
        },

        locale: function () {
            return 'fr_FR';
        },
        
        currentToolIsSelector: function () {
            return true;
        }
    };
}