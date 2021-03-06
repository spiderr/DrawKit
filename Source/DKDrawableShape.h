/**
 @author Contributions from the community; see CONTRIBUTORS.md
 @date 2005-2016
 @copyright MPL2; see LICENSE.txt
*/

#import <Cocoa/Cocoa.h>
#import "DKDrawableObject.h"

NS_ASSUME_NONNULL_BEGIN

@class DKDrawablePath, DKDistortionTransform, DKGridLayer;

//! edit operation constants tell the shape what info to display in the floater
typedef NS_ENUM(NSInteger, DKShapeEditOperation) {
	kDKShapeOperationResize = 0,
	kDKShapeOperationMove = 1,
	kDKShapeOperationRotate = 2
};

//! operation modes:
typedef NS_ENUM(NSInteger, DKShapeTransformOperation) {
	kDKShapeTransformStandard = 0, //!< normal resize/rotate transforms
	kDKShapeTransformFreeDistort = 1, //!< free distort transform
	kDKShapeTransformHorizontalShear = 2, //!< shear horizontally
	kDKShapeTransformVerticalShear = 3, //!< shear vertically
	kDKShapeTransformPerspective = 4 //!< perspective
};

/** @brief A semi-abstract base class for a drawable object consisting of any path-based shape that can be drawn to fit a rectangle.

 \c DKDrawableShape is a semi-abstract base class for a drawable object consisting of any path-based shape that can be drawn to fit a
 rectangle. DKShapeFactory can be used to supply lots of different path shapes usable with this class.

 This implements rotation of the shape about a specified point (defaulting to the centre), and also standard selection handles.

 Resizing and moving of the shape is handled by its implementations of the mouseDown/dragged/up event handlers.

 This class uses the handle drawing supplied by DKKnob.

 The path is stored internally in its untransformed form. This means that its datum is at the origin and it is unrotated. When rendered, the
 object's location and rotation angle are applied so what you see is what you expect. The bounds naturally refers to the transformed
 bounds. The selection handles themselves are also transformed by the same transform, since the shape remains editable at any orientation.

 The canonical path is set to have a bounding rect 1.0 on each side. The actual size of the object is factored into the transform to
 render the object in the drawing. Thus the original path is NEVER changed once established. This allows us to share basic shapes which
 can be generated by a factory class.
*/
@interface DKDrawableShape : DKDrawableObject <NSCoding, NSCopying, NSDraggingDestination> {
@private
	NSBezierPath* m_path; // shape's path stored in canonical form (origin centred and with unit size)
	NSMutableArray* m_customHotSpots; // list of attached custom hotspots (if any)
	DKDistortionTransform* m_distortTransform; // distortion transform for distort operations
	CGFloat m_rotationAngle; // angle of rotation of the shape
	NSPoint m_location; // where in the drawing it is placed
	NSSize m_scale; // object size/scale
	NSSize m_offset; // offset from origin of logical centre relative to canonical path
	BOOL m_hideOriginTarget; // YES to hide temporarily the origin target - done for some mouse operations
	DKShapeTransformOperation m_opMode; // drag operation mode - normal versus distortion modes
@protected
	NSRect mBoundsCache; // cached value of the bounds
	BOOL m_inRotateOp; // YES while a rotation drag is in progress
}

/** @brief Return which particular knobs are used by instances of this class.

 The default is to use all knobs, but subclasses may want to override this for particular uses.
 @return Bit mask indicating which knobs are used.
 */
@property (class) NSInteger knobMask;

@property (class) CGFloat angularConstraintAngle;

/** @brief Return the unit rect centred at the origin.
 
 This rect represents the bounds of all untransformed paths stored by a shape object.
 @return the unit rect, centred at the origin.
 */
@property (class, readonly) NSRect unitRectAtOrigin;
@property (class, strong) NSColor* infoWindowBackgroundColour;

/** @brief Return a cursor for the given partcode.
 
 Shapes have a fixed set of partcodes so the cursors can be set up by the class and cached for all
 instances. Called by the \c cursorForPartcode:mouseButtonDown: method.
 @param pc a partcode.
 @return A cursor.
 */
+ (nullable NSCursor*)cursorForShapePartcode:(NSInteger)pc;

// convenient ways to create shapes for a path you have:

/** @brief Create a shape object with the rect given.
 
 The shape's location and size is set to the rect, angle is 0 and it has the default style.
 @param aRect A rectangle.
 @return A new shape object, autoreleased.
 */
+ (DKDrawableShape*)drawableShapeWithRect:(NSRect)aRect;

/** @brief Create an oval shape object with the rect given.
 
 The shape's location and size is set to the rect, angle is 0 and it has the default style. Its path
 is an oval inscribed within the rect.
 @param aRect A rectangle.
 @return A new shape object, autoreleased.
 */
+ (DKDrawableShape*)drawableShapeWithOvalInRect:(NSRect)aRect;

/** @brief Create a shape object with the canonical path given.
 
 The path must be canonical, that is, having a bounds of {-0.5,-0.5},{1,1}. If it isn't, this
 asserts. The resulting shape must be moved, sized and rotated as required before use.
 @param path The path for the shape.
 @return A new shape object, autoreleased.
 */
+ (DKDrawableShape*)drawableShapeWithCanonicalBezierPath:(NSBezierPath*)path;

/** @brief Create a shape object with the path given
 
 The path sets the size and location of the shape. Rotation angle is set to zero.
 @param path The path for the shape.
 @return A new shape object, autoreleased.
 */
+ (DKDrawableShape*)drawableShapeWithBezierPath:(NSBezierPath*)path;

/** @brief Create a shape object with the given path and initial angle
 
 The path sets the size and location of the shape.
 @param path The path.
 @param angle Initial rotation angle.
 @return A new shape object, autoreleased.
 */
+ (DKDrawableShape*)drawableShapeWithBezierPath:(NSBezierPath*)path rotatedToAngle:(CGFloat)angle;

/** @brief Create a shape object with the given path and style
 
 The path sets the size and location of the shape, the style sets its appearance
 @param path The path.
 @param aStyle The shape's style.
 @return A new shape object, autoreleased.
 */
+ (DKDrawableShape*)drawableShapeWithBezierPath:(NSBezierPath*)path withStyle:(DKStyle*)aStyle;

/** @brief Create a shape object with the given path and initial angle and style.
 
 The path sets the size and location of the shape, the style sets its appearance.
 @param path The path.
 @param angle Initial rotation angle.
 @param aStyle The shape's style.
 @return A new shape object, autoreleased.
 */
+ (DKDrawableShape*)drawableShapeWithBezierPath:(NSBezierPath*)path rotatedToAngle:(CGFloat)angle withStyle:(DKStyle*)aStyle;

// initialise a shape for a rect or oval or different kinds of path:

/** @brief Initializes the shape to be the given rectangle.
 
 The rect establishes the shape, size and location of the shape object.
 @param instancetypeect A rectangle.
 @return The initialized object.
 */
- (instancetype)initWithRect:(NSRect)instancetypeect;

/** @brief Initializes the shape to be an oval inscribed within the given rect.
 
 The rect establishes the size and location of the shape
 @param aRect the bounding rect for an oval.
 @return The initialized object.
 */
- (instancetype)initWithOvalInRect:(NSRect)aRect;

/** @brief Initializes the shape to have the given canonical path.
 
 The resulting shape must be sized, moved and rotated as required before use. If the path passed
 is not canonical, an exception is thrown and no object is created.
 @param aPath The canonical path, that is, one having a bounds rect of size \c 1.0 centred at the origin.
 @return The initialized object
 */
- (instancetype)initWithCanonicalBezierPath:(NSBezierPath*)aPath;

/** @brief Initializes the shape to have the given path.
 
 The resulting shape is located at the centre of the path and the size is set to the width and height
 of the path's bounds. The angle is zero.
 @param aPath A path.
 @return The initialized object.
 */
- (instancetype)initWithBezierPath:(NSBezierPath*)aPath;

/** @brief Initializes the shape to have the given path.
 
 The resulting shape is located at the centre of the path and the size is set to the width and height
 of the path's bounds.
 @param aPath A path.
 @param angle The intial rotation angle of the shape, in radians.
 @return The initialized object.
 */
- (instancetype)initWithBezierPath:(NSBezierPath*)aPath rotatedToAngle:(CGFloat)angle;

// allowing style to be passed directly:

- (instancetype)initWithRect:(NSRect)aRect style:(DKStyle*)aStyle;
- (instancetype)initWithOvalInRect:(NSRect)aRect style:(DKStyle*)aStyle;
- (instancetype)initWithCanonicalBezierPath:(NSBezierPath*)aPath style:(DKStyle*)aStyle;
- (instancetype)initWithBezierPath:(NSBezierPath*)aPath style:(DKStyle*)aStyle;
- (instancetype)initWithBezierPath:(NSBezierPath*)aPath rotatedToAngle:(CGFloat)angle style:(DKStyle*)style;

// path operations:

/** @brief Sets the shape's path to be the given path.
 
 When setting, path must be bounded by the unit rect, centred at the origin. If you have some other, arbitrary path,
 the method \c adoptPath: will probably be what you want.

 When getting, the path is transformed only by any active distortion transform, but not by the shape's
 overall scale, position or rotation.
 */
@property (strong) NSBezierPath* path;

/** @brief Fetch a new path definition following a resize of the shape.
 
 @discussion
 Some shapes will need to be reshaped when their size changes. An example would be a round-cornered rect where the corners
 are expected to remain at a fixed radius whatever the shape's overall size. This means that the path needs to be reshaped
 so that the final size of the shape is used to compute the path, which is then transformed back to the internally stored
 form. This method gives a shape the opportunity to do this - it is called by the setSize method. The default method does
 nothing but subclasses can override this to implement the desired reshaping.

 Note that after reshaping, the object is refreshed automatically so you don't need to refresh it as part of this.
 */
- (void)reshapePath;

/** @brief Sets the shape's path given any path.
 
 This computes the original unit path by using the inverse transform, and sets that. Important:
 the shape's overall location should be set before calling this, as it has an impact on the
 accurate transformation of the path to the origin in the rotated case. Typically this is the
 centre point of the path, but may not be in every case, text glyphs being a prime example.
 The shape must have some non-zero size otherwise an exception is thrown.
 @param path The path to adopt.
 */
- (void)adoptPath:(NSBezierPath*)path;
/** @brief Returns the shape's path after transforming using the shape's location, size and rotation angle.
 */
@property (readonly, copy, nullable) NSBezierPath* transformedPath;
- (BOOL)canPastePathWithPasteboard:(NSPasteboard*)pb;

// geometry:

/** @brief Returns the transform representing the shape's parameters.

 This transform is local - i.e. it does not factor in the parent's transform.
 @return An autoreleased affine transform, which will convert the unit path to the final form.
 */
@property (readonly, copy) NSAffineTransform* transform;

/** @brief Returns the transform representing the shape's parameters.

 This transform is global - i.e. it factors in the parent's transform and all parents above it.
 @return An autoreleased affine transform, which will convert the unit path to the final form.
 */
@property (readonly, copy) NSAffineTransform* transformIncludingParent;

/** @brief Returns the inverse transform representing the shape's parameters.
 
 By using this method instead of inverting the transform yourself, you are insulated from optimisations
 that might be employed. Note that if the shape has no size or width, this will throw an exception
 because there is no valid inverse transform.
 @return an autoreleased affine transform, which will convert the final path to unit form.
 */
@property (readonly, copy) NSAffineTransform* inverseTransform;

/** @return a point */

/** @brief Returns the shape's current locaiton.
 @return The current location.
 */
@property (readonly) NSPoint locationIgnoringOffset;

/** @brief Interactively rotate the shape based on dragging a point.
 
 The angle of the shape is computed from the line drawn between rp and the shape's origin, allowing for
 the position of the rotation knob, and setting the shape's angle to it. \c rp is likely to be the mouse
 position while dragging the rotation knob, and the functioning of this method is based on that.
 @param rp The coordinates of a point relative to the current origin, taken to represent the rotation knob.
 @param constrain \c YES to constrain to multiples of the constraint angle, \c NO for free rotation.
 */
- (void)rotateUsingReferencePoint:(NSPoint)rp constrain:(BOOL)constrain;

/** @brief Interactively change the shape's size and/or rotation angle.

 this allows any of the main knobs (not distortion knobs) to be operated. The shape's size and/or
 angle may be affected. If the knob is a sizing knob, a constrain of YES maintains the current aspect
 ratio. If a rotate, the angle is constrained to that set by the angular constraint value. The shape's
 offset also affects this - operation are performed relative to it, so it's necessary to set the offset
 to an appropriate location prior to calling this.
 @param knobPartCode The partcode of the knob being moved.
 @param p The point that the knob should be moved to.
 @param rotate \c YES to allow any knob to rotate the shape, \c NO if only the rotate knob has this privilege.
 @param constrain \c YES to constrain appropriately, \c NO for free movement.
 */
- (void)moveKnob:(NSInteger)knobPartCode toPoint:(NSPoint)p allowRotate:(BOOL)rotate constrain:(BOOL)constrain;

/** @brief Sets the shape's offset to the location of the given knob partcode, after saving the current offset.
 
 Part of the process of setting up the interactive dragging of a sizing knob.
 @param knobPartCode A knob partcode.
 */
- (void)setDragAnchorToPart:(NSInteger)knobPartCode;

/** @brief Flip the shape horizontally.
 
 A horizontal flip is done with respect to the orthogonal drawing coordinates, based on the current
 location of the object. In fact the width and angle are simply negated to effect this.
 */
- (void)flipHorizontally;

/** @brief Set whether the shape is flipped vertically or not.
 
 A vertical flip is done with respect to the orthogonal drawing coordinates, based on the current
 location of the object. In fact the height and angle are simply negated to effect this.
 */
- (void)flipVertically;

/** @brief Resets the bounding box if the path's shape has changed.
 
 Useful after a distortion operation, this re-adopt's the shape's own path so that the effects of
 the distortion etc are retained while losing the transform itself. Rotation angle is unchanged.
 */
- (void)resetBoundingBox;

/** @brief Resets the bounding box and the rotation angle.
 
 This doesn't change the shape's appearance but readopts its current path while resetting the
 angle to zero. After a series of complex shape transformations this can be useful to realign
 the bounding box to something the user can deal with.
 */
- (void)resetBoundingBoxAndRotation;

/** @brief Adjusts location and size so that the corners lie on grid intersections if possible.
 
 This can be used to fit the object to a grid. The object's angle is not changed but its size and
 position may be. The bounding box will change but is not reset. It works by moving specific control
 points to the corners of the passed rect. Note that for rotated shapes, it's not possible to
 force the corners to lie at specific points and maintain the rectangular bounds, so the result
 may not be what you want.
 @param grid The grid to align to.
 */
- (void)adjustToFitGrid:(DKGridLayer*)grid;

/** @brief Sets whether a shape can be rotated by any knob, not just the designated rotation knob
 
 The default is <code>NO</code>, subclasses may have other ideas. Note that there are usability implications
 when returning <code>YES</code>, though the behaviour can definitely be quite useful.
 */
@property (readonly) BOOL allowSizeKnobsToRotateShape;

// operation modes:

@property (nonatomic) DKShapeTransformOperation operationMode;

// knob and partcode methods

/** @brief Given a partcode, this returns the undo action name which is the name of the action that manipulating
 that knob will cause.
 
 If your subclass uses hotspots for additional knobs, you need to override this and supply the
 appropriate string for the hotspot's action, calling super for the standard knobs.
 @param pc a knob part code
 @return a localized string, the undo action name
 */
- (NSString*)undoActionNameForPartCode:(NSInteger)pc;

/** @brief Given the partcode of a knob, this returns its current position.
 
 This is the transformed point at its true final position.
 @param knobPartCode The partcode for the knob, which is private to the shape class.
 @return The associated knob's current position.
 */
- (NSPoint)knobPoint:(NSInteger)knobPartCode;

/** @brief Gets the location of the rotation knob.
 
 Factored separately to allow override for special uses.
 @return A point, the position of the rotation knob.
 */
- (NSPoint)rotationKnobPoint;

/** @brief Draws a single knob, given its partcode.
 
 Only knobs allowed by the class mask are drawn. The knob is drawn by the \c DKKnob class attached to
 the drawing.
 @param knobPartCode The partcode for the knob, which is private to the shape class.
 */
- (void)drawKnob:(NSInteger)knobPartCode;

/** @brief Given a point in canonical coordinates (i.e. in the space {0.5,0.5,1,1}) this returns the real
 location of the point in the drawing, so applies the transforms to it, etc.
 
 This works when a distortion is being applied too, and when the shape is part of a group.
 @param rloc A point expressed in terms of the canonical rect.
 @return The same point transformed to the actual drawing.
 */
- (NSPoint)convertPointFromRelativeLocation:(NSPoint)rloc;

// distortion ops:


/** @brief The current distortion transform.
 
 Setting can be used in two ways. Either pre-prepare a transform and set it, which will immediately have
 its effect on the shape. This is the hard way. The easy way is to set the distort mode which creates
 a transform as needed and allows it to be changed interactively.

 Returns the distortion transform if there is one, or \c nil otherwise.
*/
@property (nonatomic, strong, nullable) DKDistortionTransform* distortionTransform;

// convert to editable path:

/** @brief Return a path object having the same path and style as this object.
 
 Part of the process of converting from shape to path. Both the path and the style are copied.
 @return A \c DKDrawablePath object with the same path and style as this.
 */
- (DKDrawablePath*)makePath;

/** @brief Converts each subpath in the current path to a separate object.
 
 A subpath is a path delineated by a moveTo opcode. Each one is made a separate new path. If there
 is only one subpath (common) then the result will have just one entry.
 @return An array of \c DKDrawablePath objects.
 */
- (NSArray<DKDrawableShape*>*)breakApart;

// user actions:

/** @brief Replace this object in the owning layer with a path object built from it.
 @param sender The action's sender.
 */
- (IBAction)convertToPath:(nullable id)sender;

/** @brief Set the rotation angle to zero.
 @param sender The action's sender.
 */
- (IBAction)unrotate:(nullable id)sender;

/** @brief Set the object's rotation angle from the sender's float value.
 
 Intended to be hooked up to a control rather than a menu.
 @param sender The action's sender.
 */
- (IBAction)rotate:(nullable id)sender;

/** @brief Sets the operation mode of the shape based on the sender's tag.
 @param sender The action's sender.
 */
- (IBAction)setDistortMode:(nullable id)sender;

/** @brief Resets the shape's bounding box.
 @param sender The action's sender.
 */
- (IBAction)resetBoundingBox:(nullable id)sender;
- (IBAction)toggleHorizontalFlip:(nullable id)sender;
- (IBAction)toggleVerticalFlip:(nullable id)sender;
- (IBAction)pastePath:(nullable id)sender;

@end

// part codes:

// since part codes are private to each drawable class (except 0 meaning no part), these are arranged partially as
// bit values, so they can be added together to indicate corner positions. Don't change these numbers as this
// is taken advantage of internally to simplify the handling of the part codes.

enum {
	kDKDrawableShapeLeftHandle = (1 << 0),
	kDKDrawableShapeTopHandle = (1 << 1),
	kDKDrawableShapeRightHandle = (1 << 2),
	kDKDrawableShapeBottomHandle = (1 << 3),
	kDKDrawableShapeTopLeftHandle = (1 << 4),
	kDKDrawableShapeTopRightHandle = (1 << 5),
	kDKDrawableShapeBottomLeftHandle = (1 << 6),
	kDKDrawableShapeBottomRightHandle = (1 << 7),
	kDKDrawableShapeObjectCentre = (1 << 8),
	kDKDrawableShapeOriginTarget = (1 << 9),
	kDKDrawableShapeRotationHandle = (1 << 10),
	kDKDrawableShapeTopLeftDistort = (1 << 11),
	kDKDrawableShapeTopRightDistort = (1 << 12),
	kDKDrawableShapeBottomRightDistort = (1 << 13),
	kDKDrawableShapeBottomLeftDistort = (1 << 14),
	kDKDrawableShapeSnapToPathEdge = -98,
};

// knob masks:

enum {
	kDKDrawableShapeAllKnobs = 0xFFFFFFFF,
	kDKDrawableShapeAllSizeKnobs = kDKDrawableShapeAllKnobs & ~(kDKDrawableShapeRotationHandle | kDKDrawableShapeOriginTarget | kDKDrawableShapeObjectCentre),
	kDKDrawableShapeHorizontalSizingKnobs = (kDKDrawableShapeLeftHandle | kDKDrawableShapeRightHandle | kDKDrawableShapeTopLeftHandle | kDKDrawableShapeTopRightHandle | kDKDrawableShapeBottomLeftHandle | kDKDrawableShapeBottomRightHandle),
	kDKDrawableShapeVerticalSizingKnobs = (kDKDrawableShapeTopHandle | kDKDrawableShapeBottomHandle | kDKDrawableShapeTopLeftHandle | kDKDrawableShapeTopRightHandle | kDKDrawableShapeBottomLeftHandle | kDKDrawableShapeBottomRightHandle),
	kDKDrawableShapeAllLeftHandles = (kDKDrawableShapeLeftHandle | kDKDrawableShapeTopLeftHandle | kDKDrawableShapeBottomLeftHandle),
	kDKDrawableShapeAllRightHandles = (kDKDrawableShapeRightHandle | kDKDrawableShapeTopRightHandle | kDKDrawableShapeBottomRightHandle),
	kDKDrawableShapeAllTopHandles = (kDKDrawableShapeTopHandle | kDKDrawableShapeTopLeftHandle | kDKDrawableShapeTopRightHandle),
	kDKDrawableShapeAllBottomHandles = (kDKDrawableShapeBottomHandle | kDKDrawableShapeBottomLeftHandle | kDKDrawableShapeBottomRightHandle),
	kDKDrawableShapeAllCornerHandles = (kDKDrawableShapeTopLeftHandle | kDKDrawableShapeTopRightHandle | kDKDrawableShapeBottomLeftHandle | kDKDrawableShapeBottomRightHandle),
	kDKDrawableShapeNWSECorners = (kDKDrawableShapeTopLeftHandle | kDKDrawableShapeBottomRightHandle),
	kDKDrawableShapeNESWCorners = (kDKDrawableShapeBottomLeftHandle | kDKDrawableShapeTopRightHandle),
	kDKDrawableShapeEWHandles = (kDKDrawableShapeLeftHandle | kDKDrawableShapeRightHandle),
	kDKDrawableShapeNSHandles = (kDKDrawableShapeTopHandle | kDKDrawableShapeBottomHandle)
};

NS_ASSUME_NONNULL_END
