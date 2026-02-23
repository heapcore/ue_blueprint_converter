#!/usr/bin/env python3
"""
Advanced Blueprint Generator Demo

Demonstrates procedural generation of complex Blueprints through YAML
for UE Blueprint YAML Converter.

Examples:
    python Scripts/generate_advanced_blueprint.py --type=door --interactive
    python Scripts/generate_advanced_blueprint.py --type=weapon --output=custom_weapon.yaml
    python Scripts/generate_advanced_blueprint.py --type=building --floors=5
"""

import argparse
import yaml
from pathlib import Path
from typing import Dict, Any


class AdvancedBlueprintGenerator:
    """Generator for complex Blueprint YAML definitions."""

    def __init__(self):
        self.version = 1

    def create_door_blueprint(self, **kwargs) -> Dict[str, Any]:
        """Create a door Blueprint with animation and collision."""

        door_name = kwargs.get("name", "BP_GeneratedDoor")
        has_lock = kwargs.get("has_lock", True)
        open_angle = kwargs.get("open_angle", 90.0)

        blueprint = {
            "version": self.version,
            "blueprints": [
                {
                    "name": door_name,
                    "parent": "/Script/Engine.Actor",
                    "description": f"Procedurally generated door with {'lock' if has_lock else 'no lock'}",
                    "variables": [
                        {
                            "name": "IsOpen",
                            "type": "bool",
                            "default": False,
                            "editable": True,
                            "category": "Door State",
                            "tooltip": "Whether the door is currently open",
                        },
                        {
                            "name": "OpenAngle",
                            "type": "float",
                            "default": open_angle,
                            "editable": True,
                            "category": "Door Settings",
                            "tooltip": "Angle to open the door (degrees)",
                        },
                        {
                            "name": "IsLocked",
                            "type": "bool",
                            "default": has_lock,
                            "editable": True,
                            "category": "Door State",
                            "tooltip": "Whether the door is locked",
                        },
                    ],
                    "components": [
                        {
                            "name": "RootComponent",
                            "type": "SceneComponent",
                            "transform": {
                                "location": [0, 0, 0],
                                "rotation": [0, 0, 0],
                                "scale": [1, 1, 1],
                            },
                        },
                        {
                            "name": "DoorFrame",
                            "type": "StaticMeshComponent",
                            "parent": "RootComponent",
                            "properties": {
                                "StaticMesh": "/Game/Meshes/SM_DoorFrame",
                                "Material": "/Game/Materials/M_Wood",
                            },
                        },
                        {
                            "name": "DoorPanel",
                            "type": "StaticMeshComponent",
                            "parent": "RootComponent",
                            "transform": {"location": [0, 0, 0]},
                            "properties": {
                                "StaticMesh": "/Game/Meshes/SM_DoorPanel",
                                "Material": "/Game/Materials/M_Wood",
                            },
                        },
                        {
                            "name": "CollisionBox",
                            "type": "BoxComponent",
                            "parent": "RootComponent",
                            "transform": {
                                "location": [0, 0, 100],
                                "scale": [2, 0.5, 2],
                            },
                        },
                    ],
                    "functions": [
                        {
                            "name": "OpenDoor",
                            "category": "Door Actions",
                            "access": "public",
                            "pure": False,
                            "inputs": [
                                {"name": "Speed", "type": "float", "default": 1.0},
                                {"name": "Force", "type": "bool", "default": False},
                            ],
                            "outputs": [{"name": "Success", "type": "bool"}],
                            "description": "Opens the door with specified speed",
                        },
                        {
                            "name": "CloseDoor",
                            "category": "Door Actions",
                            "access": "public",
                            "inputs": [
                                {"name": "Speed", "type": "float", "default": 1.0}
                            ],
                            "outputs": [{"name": "Success", "type": "bool"}],
                        },
                    ],
                    "events": [
                        {
                            "name": "BeginPlay",
                            "description": "Initialize door on level start",
                        },
                        {
                            "name": "OnActorBeginOverlap",
                            "description": "Handle player interaction",
                        },
                    ],
                }
            ],
        }

        # Add lock system when requested
        if has_lock:
            blueprint["blueprints"][0]["components"].append(
                {
                    "name": "LockMechanism",
                    "type": "StaticMeshComponent",
                    "parent": "DoorPanel",
                    "transform": {"location": [70, 0, 0]},
                    "properties": {"StaticMesh": "/Game/Meshes/SM_Lock"},
                }
            )

            blueprint["blueprints"][0]["functions"].append(
                {
                    "name": "ToggleLock",
                    "category": "Door Actions",
                    "access": "public",
                    "inputs": [
                        {
                            "name": "Key",
                            "type": "object",
                            "object_class": "/Game/Items/BP_Key",
                        }
                    ],
                    "outputs": [{"name": "Success", "type": "bool"}],
                }
            )

        return blueprint

    def create_weapon_blueprint(self, **kwargs) -> Dict[str, Any]:
        """Create a weapon Blueprint with effects."""

        weapon_name = kwargs.get("name", "BP_GeneratedWeapon")
        weapon_type = kwargs.get("weapon_type", "rifle")
        damage = kwargs.get("damage", 25.0)
        fire_rate = kwargs.get("fire_rate", 600.0)

        blueprint = {
            "version": self.version,
            "blueprints": [
                {
                    "name": weapon_name,
                    "parent": "/Script/Engine.Actor",
                    "description": f"Procedurally generated {weapon_type}",
                    "variables": [
                        {
                            "name": "Damage",
                            "type": "float",
                            "default": damage,
                            "editable": True,
                            "category": "Weapon Stats",
                        },
                        {
                            "name": "FireRate",
                            "type": "float",
                            "default": fire_rate,
                            "editable": True,
                            "category": "Weapon Stats",
                            "tooltip": "Rounds per minute",
                        },
                        {
                            "name": "CurrentAmmo",
                            "type": "int",
                            "default": 30,
                            "editable": False,
                            "category": "Weapon State",
                        },
                        {
                            "name": "MaxAmmo",
                            "type": "int",
                            "default": 30,
                            "editable": True,
                            "category": "Weapon Stats",
                        },
                    ],
                    "components": [
                        {"name": "RootComponent", "type": "SceneComponent"},
                        {
                            "name": "WeaponMesh",
                            "type": "StaticMeshComponent",
                            "parent": "RootComponent",
                            "properties": {
                                "StaticMesh": f"/Game/Weapons/Meshes/SM_{weapon_type.title()}"
                            },
                        },
                        {
                            "name": "MuzzlePoint",
                            "type": "SceneComponent",
                            "parent": "WeaponMesh",
                            "transform": {
                                "location": [100, 0, 0]  # Weapon muzzle
                            },
                        },
                        {
                            "name": "MuzzleFlash",
                            "type": "ParticleSystemComponent",
                            "parent": "MuzzlePoint",
                            "properties": {"Template": "/Game/Effects/P_MuzzleFlash"},
                        },
                    ],
                    "functions": [
                        {
                            "name": "Fire",
                            "category": "Weapon Actions",
                            "access": "public",
                            "inputs": [
                                {"name": "Target", "type": "vector"},
                                {"name": "Spread", "type": "float", "default": 0.0},
                            ],
                            "outputs": [
                                {"name": "Hit", "type": "bool"},
                                {"name": "HitLocation", "type": "vector"},
                            ],
                        },
                        {
                            "name": "Reload",
                            "category": "Weapon Actions",
                            "access": "public",
                            "outputs": [{"name": "Success", "type": "bool"}],
                        },
                        {
                            "name": "CanFire",
                            "category": "Weapon State",
                            "access": "public",
                            "pure": True,
                            "outputs": [{"name": "CanFire", "type": "bool"}],
                        },
                    ],
                    "events": [
                        {"name": "BeginPlay", "description": "Initialize weapon"}
                    ],
                }
            ],
        }

        return blueprint

    def create_building_blueprint(self, **kwargs) -> Dict[str, Any]:
        """Create a building Blueprint with floors."""

        building_name = kwargs.get("name", "BP_GeneratedBuilding")
        floors = kwargs.get("floors", 3)
        building_style = kwargs.get("style", "modern")

        blueprint = {
            "version": self.version,
            "blueprints": [
                {
                    "name": building_name,
                    "parent": "/Script/Engine.Actor",
                    "description": f"Procedurally generated {floors}-floor {building_style} building",
                    "variables": [
                        {
                            "name": "FloorCount",
                            "type": "int",
                            "default": floors,
                            "editable": True,
                            "category": "Building Settings",
                        },
                        {
                            "name": "FloorHeight",
                            "type": "float",
                            "default": 300.0,
                            "editable": True,
                            "category": "Building Settings",
                        },
                        {
                            "name": "BuildingStyle",
                            "type": "string",
                            "default": building_style,
                            "editable": True,
                            "category": "Building Settings",
                        },
                    ],
                    "components": [
                        {"name": "RootComponent", "type": "SceneComponent"},
                        {
                            "name": "Foundation",
                            "type": "StaticMeshComponent",
                            "parent": "RootComponent",
                            "properties": {
                                "StaticMesh": f"/Game/Buildings/{building_style}/SM_Foundation"
                            },
                        },
                    ],
                    "functions": [
                        {
                            "name": "GenerateFloors",
                            "category": "Building Generation",
                            "access": "public",
                            "description": "Procedurally generates all floors",
                        },
                        {
                            "name": "AddFloor",
                            "category": "Building Generation",
                            "access": "private",
                            "inputs": [
                                {"name": "FloorIndex", "type": "int"},
                                {"name": "Height", "type": "float"},
                            ],
                        },
                    ],
                    "events": [
                        {
                            "name": "BeginPlay",
                            "description": "Generate building structure",
                        }
                    ],
                }
            ],
        }

        # Add components for each floor
        for floor_num in range(floors):
            floor_height = floor_num * 300.0  # 300 units per floor

            blueprint["blueprints"][0]["components"].extend(
                [
                    {
                        "name": f"Floor_{floor_num + 1}",
                        "type": "StaticMeshComponent",
                        "parent": "RootComponent",
                        "transform": {"location": [0, 0, floor_height]},
                        "properties": {
                            "StaticMesh": f"/Game/Buildings/{building_style}/SM_Floor"
                        },
                    },
                    {
                        "name": f"Walls_{floor_num + 1}",
                        "type": "StaticMeshComponent",
                        "parent": f"Floor_{floor_num + 1}",
                        "properties": {
                            "StaticMesh": f"/Game/Buildings/{building_style}/SM_Walls"
                        },
                    },
                ]
            )

        return blueprint

    def interactive_generator(self):
        """Interactive mode for creating Blueprints."""
        print("Advanced Blueprint Generator")
        print("=" * 50)

        blueprint_type = input("Choose blueprint type [door/weapon/building]: ").lower()
        name = input("Blueprint name (or press Enter for default): ").strip()

        if blueprint_type == "door":
            has_lock = input("Add lock mechanism? [y/N]: ").lower().startswith("y")
            open_angle = float(input("Open angle in degrees [90]: ") or "90")

            kwargs = {
                "name": name or "BP_InteractiveDoor",
                "has_lock": has_lock,
                "open_angle": open_angle,
            }
            blueprint = self.create_door_blueprint(**kwargs)

        elif blueprint_type == "weapon":
            weapon_type = input("Weapon type [rifle/pistol/shotgun]: ") or "rifle"
            damage = float(input("Damage [25]: ") or "25")
            fire_rate = float(input("Fire rate (RPM) [600]: ") or "600")

            kwargs = {
                "name": name or f"BP_Interactive{weapon_type.title()}",
                "weapon_type": weapon_type,
                "damage": damage,
                "fire_rate": fire_rate,
            }
            blueprint = self.create_weapon_blueprint(**kwargs)

        elif blueprint_type == "building":
            floors = int(input("Number of floors [3]: ") or "3")
            style = input("Building style [modern/classic/industrial]: ") or "modern"

            kwargs = {
                "name": name or "BP_InteractiveBuilding",
                "floors": floors,
                "style": style,
            }
            blueprint = self.create_building_blueprint(**kwargs)

        else:
            print("[FAIL] Unknown blueprint type.")
            return None

        return blueprint


def main():
    parser = argparse.ArgumentParser(
        description="Generate advanced Blueprint YAML files"
    )
    parser.add_argument(
        "--type",
        choices=["door", "weapon", "building"],
        help="Type of blueprint to generate",
    )
    parser.add_argument("--name", help="Blueprint name")
    parser.add_argument("--output", help="Output YAML file")
    parser.add_argument("--interactive", action="store_true", help="Interactive mode")

    # Door-specific options
    parser.add_argument("--has-lock", action="store_true", help="Add lock to door")
    parser.add_argument(
        "--open-angle", type=float, default=90.0, help="Door open angle in degrees"
    )

    # Weapon-specific options
    parser.add_argument(
        "--weapon-type",
        default="rifle",
        choices=["rifle", "pistol", "shotgun"],
        help="Weapon type",
    )
    parser.add_argument("--damage", type=float, default=25.0, help="Weapon damage")
    parser.add_argument(
        "--fire-rate", type=float, default=600.0, help="Fire rate (RPM)"
    )

    # Building-specific options
    parser.add_argument("--floors", type=int, default=3, help="Number of floors")
    parser.add_argument(
        "--style",
        default="modern",
        choices=["modern", "classic", "industrial"],
        help="Building style",
    )

    args = parser.parse_args()

    generator = AdvancedBlueprintGenerator()

    if args.interactive:
        blueprint = generator.interactive_generator()
    else:
        if not args.type:
            print("[FAIL] Must specify --type or use --interactive mode.")
            return

        kwargs = {"name": args.name} if args.name else {}

        if args.type == "door":
            kwargs.update({"has_lock": args.has_lock, "open_angle": args.open_angle})
            blueprint = generator.create_door_blueprint(**kwargs)

        elif args.type == "weapon":
            kwargs.update(
                {
                    "weapon_type": args.weapon_type,
                    "damage": args.damage,
                    "fire_rate": args.fire_rate,
                }
            )
            blueprint = generator.create_weapon_blueprint(**kwargs)

        elif args.type == "building":
            kwargs.update({"floors": args.floors, "style": args.style})
            blueprint = generator.create_building_blueprint(**kwargs)

    if blueprint:
        if args.output:
            output_path = Path(args.output)
        else:
            blueprint_name = blueprint["blueprints"][0]["name"]
            output_path = Path(f"{blueprint_name}.yaml")

        # Write YAML
        with open(output_path, "w", encoding="utf-8") as f:
            yaml.dump(
                blueprint,
                f,
                default_flow_style=False,
                allow_unicode=True,
                sort_keys=False,
            )

        print(f"[OK] Generated: {output_path}")
        print(f"Blueprint: {blueprint['blueprints'][0]['name']}")
        print(f"Components: {len(blueprint['blueprints'][0]['components'])}")
        print(f"Functions: {len(blueprint['blueprints'][0]['functions'])}")
        print(f"Events: {len(blueprint['blueprints'][0]['events'])}")

        print("\nTo convert to Blueprint:")
        print(
            f'UnrealEditor-Cmd.exe project.uproject -run=YamlBuild -Yaml="{output_path.absolute()}"'
        )


if __name__ == "__main__":
    main()
