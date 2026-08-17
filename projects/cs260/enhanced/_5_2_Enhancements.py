import os
from pymongo import MongoClient
from bson.objectid import ObjectId

class AnimalShelter(object):
    """ CRUD operations and advanced filtering for Animal collection in MongoDB """

    def __init__(self, HOST='localhost', PORT=27017, DB='aac', COL='animals'):
        # ENHANCEMENT: Pull database credentials securely from environment variables
        USER = os.getenv('MONGO_USER', 'default_user')
        PASS = os.getenv('MONGO_PASS', 'default_pass')
        
        # Build MongoDB connection string with authentication
        connection_string = f"mongodb://{USER}:{PASS}@{HOST}:{PORT}/{DB}?authSource=admin"
        try:
            self.client = MongoClient(connection_string)
            self.database = self.client[DB]
            self.collection = self.database[COL]
        except Exception as e:
            print(f"Error connecting to MongoDB: {e}")

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

    # READ / BASIC SEARCH
    def read(self, query):
        if query is not None:
            try:
                return list(self.collection.find(query))
            except Exception as e:
                print(f"Error reading documents: {e}")
                return []
        else:
            return []

    # ENHANCEMENT: Advanced Multi-Field Filtering (Breed, Age, Status, etc.)
    def advanced_search(self, filters):
        """Allows users to query records using multiple dynamic fields at once."""
        if filters and isinstance(filters, dict):
            try:
                # Clean query by removing empty values to prevent invalid searches
                clean_query = {k: v for k, v in filters.items() if v is not None and v != ""}
                return list(self.collection.find(clean_query))
            except Exception as e:
                print(f"Error executing advanced search: {e}")
                return []
        else:
            print("Invalid or empty filter criteria provided.")
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
