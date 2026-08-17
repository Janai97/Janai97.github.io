from pymongo import MongoClient
from bson.objectid import ObjectId

class AnimalShelter(object):
    """ CRUD operations for Animal collection in MongoDB """

    def __init__(self, USER, PASS, HOST='localhost', PORT=27017, DB='aac', COL='animals'):
        # Build MongoDB connection string with authentication
        connection_string = f"mongodb://{USER}:{PASS}@{HOST}:{PORT}/{DB}?authSource=admin"
        self.client = MongoClient(connection_string)
        self.database = self.client[DB]
        self.collection = self.database[COL]

    # CREATE
    def create(self, data):
        if data:
            try:
                result = self.collection.insert_one(data)
                return True if result.inserted_id else False
            except Exception as e:
                print(f"Error inserting document: {e}")
                return False
        else:
            raise Exception("Nothing to save, data is empty")

    # READ
    def read(self, query):
        if query is not None:
            try:
                return list(self.collection.find(query))
            except Exception as e:
                print(f"Error reading documents: {e}")
                return []
        else:
            return []

    # UPDATE
    def update(self, query, new_values):
        if query and new_values:
            try:
                result = self.collection.update_many(query, {"$set": new_values})
                return result.modified_count
            except Exception as e:
                print(f"Error updating documents: {e}")
                return 0
        else:
            raise Exception("Update requires both query and new values")

    # DELETE
    def delete(self, query):
        if query:
            try:
                result = self.collection.delete_many(query)
                return result.deleted_count
            except Exception as e:
                print(f"Error deleting documents: {e}")
                return 0
        else:
            raise Exception("Delete requires a query")
