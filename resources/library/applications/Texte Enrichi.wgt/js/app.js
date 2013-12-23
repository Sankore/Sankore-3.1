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
                    onLinkClick: function (a) {},
                    onInit: function () {},
                    onBlur: function () {}
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
                        plugins: ['link', 'searchreplace', 'table', 'paste', 'textcolor', 'rteditor'],
                        toolbar1: 'bold italic underline strikethrough | forecolor backcolor pagecolor | link | undo redo',
                        toolbar2: 'fontselect fontsizeselect  | alignleft aligncenter alignright alignjustify | customtable'
                    };

                options.fontsize_formats = [
                    '6pt', '7pt', '8pt', '9pt', '10pt', '11pt', '12pt',
                    '14pt', '16pt', '18pt', '20pt', '22pt', '24pt', '26pt', '28pt',
                    '36pt', '48pt', '72pt'
                ].join(' ');

                options.font_formats = [
                    'Alphonetic=alphonetic,serif',
                    'Alphonetic GB=alphoneticGB,serif',
                    'Andika Basic=andika basic,sans',
                    'Arial=arial,sans',
                    'Arial Black=arial black,sans',
                    'Comic Sans MS=comic sans ms,sans',
                    'Courier New=courier new,courier,serif',
                    'Cursive Standard=cursive standard,serif',
                    'Écolier=ecolier,serif',
                    'Écolier (court)=ecolier_court,serif',
                    'Écolier CP=ecolier_cp,serif',
                    'Écolier CP (pointillés)=ecolier_cp_pointillés,serif',
                    'Écolier lignes=ecolier_lignes,serif',
                    'Écolier lignes (court)=ecolier_lignes_court,serif',
                    'Écolier lignes (pointillés)=ecolier_lignes_pointillés,serif',
                    'Écolier (pointillés)=ecolier_pointillés,serif',
                    'Écriture A=ecriture a,serif',
                    'Écriture B=ecriture b,serif',
                    'Georgia=georgia,serif',
                    'Gino School Script=ginoschoolscript,serif',
                    'Impact=impact,sans',
                    'Script École=script cole,serif',
                    'Script École 2=script ecole 2,serif',
                    'Scriptcase cole=scriptcase cole,serif',
                    'Times New Roman=times new roman,times,serif',
                    'Trebuchet MS=trebuchet ms,sans',
                    'Verdana=verdana,serif'
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

                    editor.on('show', function (e) {
                        self.onTinyShow(e);
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
                            editor.menuItems.row
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
                    self.options.onLinkClick.call(self, e);    
                });

                if (this.widget) {
                    this.widget.onfocus = function () {
                        if (window && window.sankore && window.sankore.currentToolIsSelector()) {
                            self.show();

                            if (self.empty) {
                                self.setContent('');
                            }

                            $('body').removeClass('view');
                        }
                    };

                    this.widget.onblur = function () {
                        self.tinymce.fire('blur');

                        var content = self.getContent();

                        if (self.empty = !content.trim().length) {
                            self.setContent(tinymce.translate(self.options.defaultText));
                        }

                        self.hide();

                        $('body').addClass('view');
                    };

                    this.widget.ondrop = function (url, contentType) {
                        if (self.isAllowedDroppedContentType(contentType)) {
                            self.addDroppedContent(url);
                        }
                    };
                }
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
            };

            /**
             * Hide the editor
             */
            app.RTEditor.prototype.hide = function () {
                this.tinymce.hide();
            };

            /**
             * Change the background independently to the dark background feature
             */
            app.RTEditor.prototype.setBackgroundColor = function (color) {
                if ((color === '#FFFFFF' && !this.dark) || (color === '#000000' && this.dark)) {
                    color = 'transparent';
                }

                this.backgroundColor = color;

                $(this.tinymce.getDoc()).find('body').css('background-color', color);
                this.$container.css('background-color', color);
            };

            /**
             * Switch background to inverted colors (preserving color other than white and black)
             */
            app.RTEditor.prototype.setDarkBackground = function (dark) {
                dark = !! dark;

                var $docBody = $(this.tinymce.getDoc()).find('body'),
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
                    cleanColorSpan($docBody, 'rgb(0, 0, 0)');
                } else {
                    $docBody.removeClass(darkClassname);
                    this.$container.removeClass(darkClassname);
                    cleanColorSpan($docBody, 'rgb(255, 255, 255)');
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
             * Event handler for TinyMCE editor 'init' event
             */
            app.RTEditor.prototype.onTinyInit = function (e) {
                this.refreshSize();
                this.options.onInit.call(this);

                var editor = this.tinymce;

                this.tinymce.editorCommands.addCommands({
                    'HiliteColor': function (command, ui, value) {
                        var set = false;
                        if (editor.selection.selectedRange && editor.selection.selectedRange.commonAncestorContainer) {
                            var $container = $(editor.selection.selectedRange.commonAncestorContainer),
                                tds = [],
                                all = $container.find('*'),
                                i;

                            for (i = 0; i < all.length; i++) {
                                if (editor.selection.getSel().containsNode(all[i], true) && $(all[i]).is('td')) {
                                    tds.push(all[i]);
                                }
                            }

                            if ($container.is('td')) {
                                tds.push($container.get('0'));
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
                
                var confirm = this.tinymce.windowManager.confirm;
                this.tinymce.windowManager.confirm = function (message, callback, scope) {
                    if (message.indexOf('external link') !== -1) {
                        callback.call(scope || this, true);
                    } else {
                        confirm.call(this, message, callback, scope);
                    }
                };
            };

            /**
             * Event handler for TinyMCE editor 'blur' event
             */
            app.RTEditor.prototype.onTinyBlur = function (e) {
                this.options.onBlur.call(this);
            };

            /**
             * Event handler for TinyMCE editor 'show' event
             */
            app.RTEditor.prototype.onTinyShow = function (e) {
                this.tinymce.focus();
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

            app.RTEditor.defined = true;
        }

        try {
            this.init(id, options);
        } catch (e) {
            console.error(e.stack);
        }
    };

    $(document).ready(function () {
        var options = {
            locale: 'en_GB',
            defaultText: 'Insert your text here'
        }, widget = null;

        if (window.sankore) {
            options.onLinkClick = function (a) {
                window.sankore.loadUrl($(a).attr('href'));
            };

            options.onInit = function () {
                var text = window.sankore.preference('content', '');

                this.setContent(text);

                if (text.trim().length !== 0) {
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

            options.locale = window.sankore.locale();
        } else {
            options.onLinkClick = function (a) {
                console.log(a);
            };
        }

        if (window.widget) {
            widget = window.widget;
        }

        window.rteditor = new app.RTEditor('ubwidget', widget, options);
    });
}(jQuery));