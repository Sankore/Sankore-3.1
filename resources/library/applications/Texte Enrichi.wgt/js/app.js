/*jslint browser: true, devel: true, plusplus: true, nomen: true*/
/*global tinymce: true, jQuery: true*/
(function ($) {
    "use strict";
    var app = {
        allowedLangs: ['fr_FR', 'en_GB']
    };

    app.RTEditor = function (id, widget, options) {

        var allowedDroppedContentTyped = [
            'image/png', 'image/jpg', 'image/jpeg', 'image/gif', 'image/svg+xml', 'image/tiff'
        ];

        var resizeTimer = null;

        this.id = id;
        this.empty = true;
        this.font = {
            name: 'arial',
            size: '32pt',
            bold: false,
            italic: false
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
                    onFirstType: function () {},
                    onContentOverflow: function (delta) {},
                    onFontSizeChange: function (fontSize) {},
                    onFontFamilyChange: function (fontFamily) {},
                    onFontFormatChange: function (name, value) {},
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
                        convert_newlines_to_br: false,
                        remove_linebreaks: false,
                        valid_elements: '*[*]',
                        keep_styles: true,
                        plugins: ['link', 'table', 'paste', 'textcolor', 'rteditor', 'code'],
                        toolbar1: 'bold italic underline strikethrough | forecolor backcolor pagecolor | bullist numlist | outdent indent | link | customtable',
                        toolbar2: 'fontselect fontsizeselect increasefontsize decreasefontsize | alignleft aligncenter alignright alignjustify | undo redo | code'
                    };

                options.fontsize_formats = [
                    '12pt', '14pt', '16pt', '18pt', '20pt', '22pt', '24pt',
                    '26pt', '28pt', '36pt', '48pt', '72pt', '96pt', '128pt'
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
                    'Écolier (court)=ecolier_court',
                    'Écolier CP=ecolier_cp',
                    'Écolier CP (pointillés)=ecolier_cp_pointillés',
                    'Écolier lignes (court)=ecolier_lignes_court',
                    'Écolier lignes (pointillés)=ecolier_lignes_pointillés',
                    'Écolier (pointillés)=ecolier_pointillés',
                    'Écriture A=ecriture a',
                    'Écriture A Ligne=ecriture a ligne',
                    'Écriture A Orné=ecriture a orne',
                    'Écriture A Orné Ligne=ecriture a orne ligne',
                    'Écriture B=ecriture b',
                    'Écriture B Ligne=ecriture b ligne',
                    'Écriture B Orné=ecriture b orne',
                    'Écriture B Orné Ligne=ecriture b orne ligne',
                    'Georgia=georgia',
                    'Gino School Script=ginoschoolscript',
                    'Impact=impact',
                    'Script École 2=script ecole 2',
                    'Scriptcase cole=scriptcase cole',
                    'Times New Roman=times new roman',
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

                    editor.on('keypress', function (e) {
                        if (app.RTEditor.isContentEmpty($(e.target).html())) {
                            self.tinymce.setContent('', {
                                format: 'raw'
                            });
                            self.applyDefaults();
                        }
                    });

                    editor.on('NodeChange', function (e) {
                        if (e.element.nodeName === 'TD' && e.element.innerHTML.length === 0) {
                            e.element.appendChild(document.createElement('br'));
                        }
                    });

                    editor.on('PreProcess', function (e) {
                        $(e.node).find('br[data-mce-bogus]').replaceWith('<br>');
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

                    var fonts = {
                        list: editor.settings.fontsize_formats.split(' '),
                        indexOf: function (font) {
                            var index = this.list.indexOf(font);
                            return index === -1 ? undefined : index;
                        },
                        at: function (index) {
                            return this.list[index] || null;
                        },
                        prev: function (font) {
                            return this.at(this.indexOf(font) - 1);
                        },
                        next: function (font) {
                            return this.at(this.indexOf(font) + 1);
                        }
                    };

                    var getFontSizeAtCursor = function () {
                        var container = editor.selection.getRng().startContainer,
                            $el;

                        if (container.nodeType === 3) {
                            $el = $(container.parentElement);
                        } else {
                            $el = $(container);
                        }

                        return Math.round(Number($el.css('font-size').replace('px', '')) * 0.75) + 'pt';
                    };

                    editor.addButton('increasefontsize', {
                        type: 'button',
                        text: false,
                        icon: 'increasefontsize',
                        onclick: function () {
                            var fontSize = getFontSizeAtCursor();
                            if (fontSize && fonts.next(fontSize)) {
                                editor.execCommand('FontSize', true, fonts.next(fontSize));
                            }
                        }
                    });

                    editor.addButton('decreasefontsize', {
                        type: 'button',
                        text: false,
                        icon: 'decreasefontsize',
                        onclick: function () {
                            var fontSize = getFontSizeAtCursor();
                            if (fontSize && fonts.prev(fontSize)) {
                                editor.execCommand('FontSize', true, fonts.prev(fontSize));
                            }
                        }
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

            app.RTEditor.prototype.applyDefaults = function () {
                if (!this.tinymce.queryCommandValue('FontName')) {
                    this.tinymce.execCommand('FontName', true, this.font.name);
                }

                if (!this.tinymce.queryCommandValue('FontSize')) {
                    this.tinymce.execCommand('FontSize', true, this.font.size);
                }

                if (this.font.bold) {
                    this.tinymce.execCommand('Bold', true);
                }

                if (this.font.italic) {
                    this.tinymce.execCommand('Italic', true);
                }
                
                var p = this.tinymce.getDoc().querySelectorAll('p > br:only-child').item(0);
                p.parentElement.removeChild(p);
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
                        self.tinymce.save();
                        self.tinymce.fire('blur');

                        var content = self.getContent();

                        self.empty = app.RTEditor.isContentEmpty(content);

                        if (self.empty) {
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
                $('.mcs-resizehandle', this.tinymce.getDoc()).css('display', 'block');
                $('.mce-toolbar-grp').css('visibility', 'visible');
                this.tinymce.getDoc().body.contentEditable = true;
                this.tinymce.getDoc().designMode = 'on';
                this.tinymce.settings.object_resizing = true;
            };

            /**
             * Hide the editor
             */
            app.RTEditor.prototype.hide = function () {
                this.tinymce.selection.collapse();
                $('.mce-resizehandle', this.tinymce.getDoc()).css('display', 'none');
                $('.mce-floatpanel').css('display', 'none');
                $('.mce-toolbar-grp').css('visibility', 'hidden');
                this.tinymce.getDoc().body.contentEditable = false;
                this.tinymce.getDoc().designMode = 'off';
                this.tinymce.settings.object_resizing = false;
            };

            /**
             * Change the background independently to the dark background feature
             */
            app.RTEditor.prototype.setBackgroundColor = function (color) {
                if ((color === '#FFFFFF' && !this.dark) || (color === '#000000' && !this.dark)) {
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
                dark = !! dark;

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
                return this.tinymce.getContent({
                    format: 'raw'
                });
            };

            /**
             *
             */
            app.RTEditor.prototype.setContent = function (content) {
                var $content = $('<div>' + content + '</div>');

                $content.find('td,p').each(function () {
                    if (this.innerHTML.length === 0) {
                        this.appendChild(document.createElement('br'));
                    }
                });

                this.tinymce.setContent($content.html(), {
                    format: 'raw'
                });
                this.tinymce.save();
            };

            /**
             *
             */
            app.RTEditor.prototype.setDefaultFontFamily = function (name) {
                this.tinymce.getDoc().body.style.fontFamily = name;
                if (name.length > 0) {
                    this.font.name = name;
                }
            };

            /**
             *
             */
            app.RTEditor.prototype.setDefaultFontSize = function (size) {
                size = size.replace('pt', '');
                if (!isNaN(Number(size)) && Number(size) > 0) {
                    this.tinymce.getDoc().body.style.fontSize = size + 'pt';
                    this.font.size = size + 'pt';
                }
            };

            /**
             *
             */
            app.RTEditor.prototype.setDefaultFontBold = function (isBold) {
                this.font.bold = isBold;
            };

            /**
             *
             */
            app.RTEditor.prototype.setDefaultFontItalic = function (isItalic) {
                this.font.italic = isItalic;
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
                var self = this,
                    delta = $(self.tinymce.getDoc()).height() - self.getIframe().height();

                if (delta > 0) {
                    if (self.resizeTimeout !== null) {
                        clearTimeout(this.resizeTimeout);
                    }

                    self.resizeTimeout = setTimeout(function () {
                        self.options.onContentOverflow.call(self, $(self.tinymce.getDoc()).height() - self.getIframe().height());
                        self.resizeTimeout = null;
                    }, 0);
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

                $(editor.getDoc()).bind('keydown', function (e) {
                    self.checkForResize();
                });

                // hack dégueulasse pour se passer de confirmation dans le cas d'ajout de lien externe
                var confirm = this.tinymce.windowManager.confirm;
                this.tinymce.windowManager.confirm = function (message, callback, scope) {
                    if (message.indexOf('external link') !== -1) {
                        callback.call(scope || this, true);
                    } else {
                        confirm.call(this, message, callback, scope);
                    }
                };

                if (!this.options.autoShow) {
                    this.hide();
                } else {
                    if (app.RTEditor.isContentEmpty(this.getContent())) {
                        this.applyDefaults();
                        this.tinymce.getDoc().body.click();
                    }
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
                    this.options.onFontFormatChange.call(this, e.value, this.tinymce.queryCommandState(e.value));
                }

                if (e.command === 'FontName') {
                    this.options.onFontFamilyChange.call(this, e.value);
                    this.font.name = e.value;
                }

                if (e.command === 'FontSize') {
                    this.options.onFontSizeChange.call(this, e.value);

                    var cac = this.tinymce.selection.explicitRange.commonAncestorContainer,
                        cacp = cac.parentElement,
                        cacpp = cacp.parentElement;

                    if (cacpp && cacpp.nodeName === 'LI' && cac.nodeType === 3) {
                        cacpp.style.fontSize = e.value;
                    } else if (cacpp && 'UL,OL'.indexOf(cacpp.nodeName) !== -1 && this.tinymce.selection.explicitRange.endContainer.nodeType === 3) {
                        cacp.style.fontSize = e.value;
                    }

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

                if (this.font.bold) {
                    $table.css('font-weight', 'bold');
                }

                if (this.font.italic) {
                    $table.css('font-style', 'italic');
                }
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
        var $content = $('<div>' + content + '</div>');
        return $content.text().trim().length === 0 && $content.find('img,table').length === 0;
    };

    $(document).ready(function () {
        var options = {
            locale: 'en_GB',
            defaultText: 'Insert your text here',
            autoShow: false
        }, widget = null;

        if (window.sankore) {
            options.onLinkClick = function (a) {
                var href = $(a).attr('href'),
                    prefix = 'http';

                if (href.substring(0, prefix.length) !== prefix) {
                    href = prefix + '://' + href;
                }

                window.sankore.loadUrl(href);
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

                if (window.sankore.preference('font')) {
                    var font = JSON.parse(window.sankore.preference('font'));
                    console.log(font);
                    this.setDefaultFontFamily(font.name);
                    this.setDefaultFontSize(font.size);
                    this.setDefaultFontBold(font.bold);
                    this.setDefaultFontItalic(font.italic);
                } else {
                    this.setDefaultFontFamily(window.sankore.fontFamilyPreference());
                    this.setDefaultFontSize(window.sankore.fontSizePreference());
                    this.setDefaultFontBold(window.sankore.fontBoldPreference());
                    this.setDefaultFontItalic(window.sankore.fontItalicPreference());
                }
            };

            options.onBlur = function () {
                window.sankore.setPreference('content', this.getContent());
                window.sankore.setPreference('dark', this.dark ? 'true' : 'false');
                window.sankore.setPreference('background', this.backgroundColor);
                window.sankore.setPreference('font', JSON.stringify(this.font));
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

            options.onFontFormatChange = function (name, value) {
                if (name === 'bold') {
                    window.sankore.updateFontBoldPreference(value);
                } else if (name === 'italic') {
                    window.sankore.updateFontItalicPreference(value);
                }
            };

            if (app.allowedLangs.indexOf(window.sankore.locale()) !== -1) {
                options.locale = window.sankore.locale();
            }
            options.dark = window.sankore.isDarkBackground();
        }

        if (window.widget) {
            widget = window.widget;

            widget.onChangeBackground = function () {
                if (window.sankore && widget.onbackgroundswitch) {
                    widget.onbackgroundswitch(window.sankore.isDarkBackground());
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

/** mock widget object for browser testing */
if (!('widget' in window)) {
    window.widget = {};
}

/** mock sankore object for browser testing */
if (!('sankore' in window)) {
    var preferences = {
        content: ''
    };

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
            return 'arial';
        },

        fontSizePreference: function () {
            return '24pt';
        },

        updateFontSizePreference: function (name) {
            console.log('Default font size set to : ' + name);
        },

        updateFontBoldPreference: function (bold) {
            console.log('Default bold set to : ', bold);
        },

        fontBoldPreference: function () {
            return true;
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