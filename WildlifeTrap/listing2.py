# search for userMotionCodeHere. There will be 2 results, edit the second so you are passing filename to the function
userMotionCodeHere(filename)

# modules imported within this code as it didn't seem to like only being imported in the pi-timolo code
import io
import tweepy
import time
from google.cloud import vision
from google.cloud.vision import types
from google.cloud import storage

# make sure you include filename as a parameter in the function
# Changed the name of the function to match that in pi-timolo code
def userMotionCode(filenamePath):
    # the following code until the Google Vision API is taken from the pi-timolo user_motion_code template
    """
    Users can put code here that needs to be run
    after motion detected and image/video taken
    Eg Notify or activate something.

    Note all functions and variables will be imported.
    pi-timolo.py will execute this function userMotionCode(filename)
    in pi-timolo.py per example below

        user_motion_code.userMotionCode(filename)

    """
    # Insert User code Below:
    print("User Code Executing from userMotionCode function")
    print("file path is %s" % filenamePath)
    
    # we need to create an instance of the Google Vision API
    client = storage.Client()
    # instantiates a client
    client = vision.ImageAnnotatorClient()

    # loads the image into memory
    with io.open(filename, 'rb') as image_file:
        content = image_file.read()

    image = types.Image(content=content)

    # performs label detection on the image file
    response = client.label_detection(image=image)
    # pass the response into a variable
    labels = response.label_annotations
    
    # we have our labels, now create a string to add to the tweet message
    # for debugging - let’s see what Google thinks is in the image
    print('Labels:')
    # add labels to our tweet text
    tweetText = "Labels: "
    animalInPic = False
    for label in labels:
        print(label.description)
        tweetText = tweetText + " " + label.description
        # edit this line to change the animal you want to detect
        if "Bird" in tweetText: animalInPic = True # capitalised Bird and True

    # set up Tweepy
    # consumer keys and access tokens, used for authorisation  
    consumer_key = ‘XXX’  
    consumer_secret = ‘XXX’  
    access_token = ‘XXX’  
    access_token_secret = ‘XXX’

    # authorisation process, using the keys and tokens  
    auth = tweepy.OAuthHandler(consumer_key, consumer_secret)  
    auth.set_access_token(access_token, access_token_secret)  
      
    # creation of the actual interface, using authentication  
    api = tweepy.API(auth)  

    # send the tweet with photo and message 
    photo_path = filename
    # only send tweet if it contains a desired animal
    if animalInPic:
        api.update_with_media(photo_path, status=tweetText)
        time.sleep(3600) # 1 hour sleep timer to reduce bird spam on twitter feed after feedback from followers
    
    return
