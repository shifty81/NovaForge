using System.Collections.Generic;
using NovaForge.Data.Models;

namespace NovaForge.Game.Crafting
{
    /// <summary>
    /// Pure-logic crafting helpers. All state lives in the caller's <see cref="Inventory"/>;
    /// this class has no state of its own and is fully unit-testable.
    /// </summary>
    public static class CraftingSystem
    {
        /// <summary>
        /// Attempts to craft one output of the recipe identified by <paramref name="recipeId"/>.
        /// Deducts ingredients from <paramref name="inventory"/> on success.
        /// </summary>
        /// <returns>True if the recipe was found and all ingredients were available; false otherwise.</returns>
        public static bool TryCraft(string recipeId, Inventory inventory, List<CraftingRecipe> recipes)
        {
            var recipe = recipes.Find(r => r.Id == recipeId);
            if (recipe == null) return false;
            if (!inventory.TryRemoveItems(recipe.Ingredients)) return false;
            inventory.AddItem(recipe.OutputId, recipe.OutputCount);
            return true;
        }

        /// <summary>Returns all recipes whose ingredient requirements are currently met by <paramref name="inventory"/>.</summary>
        public static List<CraftingRecipe> GetCraftableRecipes(Inventory inventory, List<CraftingRecipe> recipes)
        {
            var result = new List<CraftingRecipe>();
            foreach (var recipe in recipes)
            {
                bool canCraft = true;
                foreach (var kv in recipe.Ingredients)
                {
                    if (inventory.GetCount(kv.Key) < kv.Value) { canCraft = false; break; }
                }
                if (canCraft) result.Add(recipe);
            }
            return result;
        }
    }
}
