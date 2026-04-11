using System.Collections.Generic;
using Xunit;
using NovaForge.Data.Models;
using NovaForge.Game;
using NovaForge.Game.Crafting;

namespace NovaForge.Tests
{
    public class CraftingSystemTests
    {
        private static List<CraftingRecipe> MakeRecipes()
        {
            return new List<CraftingRecipe>
            {
                new CraftingRecipe
                {
                    Id = "iron_plate",
                    Name = "Iron Plate",
                    Ingredients = new Dictionary<string, int> { ["iron_scrap"] = 3 },
                    OutputId = "iron_plate",
                    OutputCount = 1
                },
                new CraftingRecipe
                {
                    Id = "refined_ore",
                    Name = "Refined Ore",
                    Ingredients = new Dictionary<string, int> { ["rare_ore"] = 2 },
                    OutputId = "refined_ore",
                    OutputCount = 1
                }
            };
        }

        [Fact]
        public void TryCraft_SucceedsAndAddsOutput_WhenIngredientsAvailable()
        {
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 5);
            var recipes = MakeRecipes();

            bool result = CraftingSystem.TryCraft("iron_plate", inv, recipes);

            Assert.True(result);
            Assert.Equal(1, inv.GetCount("iron_plate"));
        }

        [Fact]
        public void TryCraft_DeductsIngredients_OnSuccess()
        {
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 5);
            var recipes = MakeRecipes();

            CraftingSystem.TryCraft("iron_plate", inv, recipes);

            Assert.Equal(2, inv.GetCount("iron_scrap")); // 5 - 3 = 2
        }

        [Fact]
        public void TryCraft_ReturnsFalse_WhenInsufficientIngredients()
        {
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 2); // need 3
            var recipes = MakeRecipes();

            bool result = CraftingSystem.TryCraft("iron_plate", inv, recipes);

            Assert.False(result);
        }

        [Fact]
        public void TryCraft_DoesNotModifyInventory_WhenInsufficientIngredients()
        {
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 2);
            var recipes = MakeRecipes();

            CraftingSystem.TryCraft("iron_plate", inv, recipes);

            Assert.Equal(2, inv.GetCount("iron_scrap"));
            Assert.Equal(0, inv.GetCount("iron_plate"));
        }

        [Fact]
        public void TryCraft_ReturnsFalse_WhenRecipeNotFound()
        {
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 10);
            var recipes = MakeRecipes();

            bool result = CraftingSystem.TryCraft("nonexistent_recipe", inv, recipes);

            Assert.False(result);
        }

        [Fact]
        public void TryCraft_RespectsOutputCount()
        {
            var recipes = new List<CraftingRecipe>
            {
                new CraftingRecipe
                {
                    Id = "bulk_stone",
                    Ingredients = new Dictionary<string, int> { ["stone"] = 1 },
                    OutputId = "stone_slab",
                    OutputCount = 4
                }
            };
            var inv = new Inventory();
            inv.AddItem("stone", 1);

            CraftingSystem.TryCraft("bulk_stone", inv, recipes);

            Assert.Equal(4, inv.GetCount("stone_slab"));
        }

        [Fact]
        public void GetCraftableRecipes_ReturnsOnlyAffordableRecipes()
        {
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 3); // enough for iron_plate but not refined_ore
            var recipes = MakeRecipes();

            var craftable = CraftingSystem.GetCraftableRecipes(inv, recipes);

            Assert.Single(craftable);
            Assert.Equal("iron_plate", craftable[0].Id);
        }

        [Fact]
        public void GetCraftableRecipes_ReturnsEmpty_WhenInventoryEmpty()
        {
            var inv = new Inventory();
            var craftable = CraftingSystem.GetCraftableRecipes(inv, MakeRecipes());
            Assert.Empty(craftable);
        }

        [Fact]
        public void GetCraftableRecipes_ReturnsAll_WhenAllAffordable()
        {
            var inv = new Inventory();
            inv.AddItem("iron_scrap", 10);
            inv.AddItem("rare_ore", 10);

            var craftable = CraftingSystem.GetCraftableRecipes(inv, MakeRecipes());

            Assert.Equal(2, craftable.Count);
        }
    }
}
