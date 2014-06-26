/* jshint browser:true */
/* globals klass:true, Sankore:true */
(function () {
    "use strict";

    klass.define('Sankore.UI', 'DragDropManager', klass.extend({
        constructor: function (eventDispatcher) {
            this.eventDispatcher = eventDispatcher;
            this.drag = null;
            this.ghost = null;

            this.attachEventHandlers();
        },

        /**
         *
         */
        attachEventHandlers: function () {
            var self = this;

            document.addEventListener('mouseup', function (e) {
                if (self.isDragging()) {
                    self.endDrag();
                }
            });

            document.addEventListener('mousemove', function (e) {
                if (self.isDragging() && self.ghost) {
                    self.moveGhostObject(e.x, e.y);
                }
            });
        },

        /**
         * @param {HTMLElement} element
         */
        registerDraggable: function (element, group) {
            var self = this;
            
            element.addEventListener('mousedown', function (e) {
                if (self.isDraggable(element)) {
                    self.startDrag(element);
                }
            });

            if (typeof group === 'undefined' || !group) {
                group = '_global';
            }

            element.dataset.draggable = 'true';
            element.dataset.dragGroup = group;
        },

        /**
         * @param {HTMLElement} element
         */
        registerDropzone: function (element, group) {
            var self = this;

            element.addEventListener('mouseup', function (e) {
                e.stopPropagation();

                if (self.isDroppable(element)) {
                    self.endDrag(element);
                }
            });

            element.addEventListener('mouseover', function (e) {
                if (self.isDroppable(element)) {
                    self.fireDragEnter(element);
                }
            });

            element.addEventListener('mouseout', function (e) {
                if (self.isDroppable(element)) {
                    self.fireDragLeave(element);
                }
            });

            if (typeof group === 'undefined' || !group) {
                group = '_global';
            }

            element.dataset.droppable = 'true';
            element.dataset.dragGroup = group;
        },
        
        /**
         * @param {HTMLElement} element
         * @returns {Boolean}
         */
        isDraggable: function (element) {
            if (this.isDragging()) {
                return false;
            }
            
            return 'draggable' in element.dataset && element.dataset.draggable === 'true';
        },
        
        /**
         * @param {HTMLElement} element
         * @returns {Boolean}
         */
        isDroppable: function (element) {
            if (!this.isDragging()) {
                return false;
            }
            
            return 'droppable' in element.dataset && element.dataset.droppable === 'true';
        },

        /**
         * @returns {Boolean}
         */
        isDragging: function () {
            return this.drag !== null;
        },

        /**
         * @param {HTMLElement} element
         */
        startDrag: function (source) {
            this.drag = {
                element: source
            };

            this.createGhostObject(source);

            this.eventDispatcher.notify('dragdrop_manager.dragstart', {
                source: source,
                group: source.dataset.dragGroup
            });
        },

        /**
         * @param {HTMLElement} element
         */
        createGhostObject: function (element) {
            this.ghost = element.cloneNode(true);
            this.ghost.style.position = 'absolute';
            this.ghost.style.pointerEvents = 'none';

            this.moveGhostObject(element.offsetLeft, element.offsetTop);

            element.parentElement.insertBefore(this.ghost, element.parentElement.firstChild);
        },

        /**
         * @param {Number} x
         * @param {Number} y
         */
        moveGhostObject: function (x, y) {
            var offset = [
                this.ghost.clientWidth / 2,
                this.ghost.clientHeight / 2
            ];
            
            this.ghost.style.left = (Math.max(0, Math.min(window.innerWidth - offset[0] * 2, x - offset[0]))) + 'px';
            this.ghost.style.top = (Math.max(0, Math.min(window.innerHeight - offset[1] * 2, y - offset[1]))) + 'px';
        },

        /**
         *
         */
        removeGhostObject: function () {
            if (this.ghost.parentElement) {
                this.ghost.parentElement.removeChild(this.ghost);
            }
            
            this.ghost = null;
        },

        /**
         * @param {HTMLElement} destination
         */
        endDrag: function (destination) {
            this.eventDispatcher.notify('dragdrop_manager.dragend', {
                source: this.drag.element,
                group: this.drag.element.dataset.dragGroup
            });

            if (destination && this.drag.element.dataset.dragGroup === destination.dataset.dragGroup) {
                this.eventDispatcher.notify('dragdrop_manager.drop', {
                    source: this.drag.element,
                    destination: destination,
                    group: this.drag.element.dataset.dragGroup
                });

                this.fireDragLeave(destination);
            }

            this.removeGhostObject();

            this.drag = null;
        },

        /**
         * @param {HTMLElement} zone
         */
        fireDragEnter: function (zone) {
            if (this.drag.element.dataset.dragGroup === zone.dataset.dragGroup) {
                this.eventDispatcher.notify('dragdrop_manager.dragenter', {
                    source: this.drag.element,
                    zone: zone,
                    group: this.drag.element.dataset.dragGroup
                });
            }
        },

        /**
         * @param {HTMLElement} zone
         */
        fireDragLeave: function (zone) {
            if (this.drag.element.dataset.dragGroup === zone.dataset.dragGroup) {
                this.eventDispatcher.notify('dragdrop_manager.dragleave', {
                    source: this.drag.element,
                    zone: zone,
                    group: this.drag.element.dataset.dragGroup
                });
            }
        }
    }));
})();